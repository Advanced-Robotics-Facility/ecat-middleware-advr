#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <csignal>
#include <string>


#include "advrf_cyclonedds_plugin/publisher.hpp"

#include <ecat_master_future/shm_utils.hpp> 
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/pdo_utils.hpp>

namespace {
volatile std::sig_atomic_t keep_running = 1;

void on_signal(int)
{
    keep_running = 0;
}

std::unique_ptr<SharedMemoryClient> wait_for_shared_memory()
{
    while (keep_running) {
        auto shm = std::make_unique<SharedMemoryClient>(SHM_NAME, sizeof(SharedBridge));
        if (shm->is_valid())
            return shm;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return nullptr;
}
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;

    clock_utils::init();
    std::cout << "[SHM-DDS Bridge] Connecting to shared memory: " << SHM_NAME << std::endl;

    auto shm = wait_for_shared_memory();
    if (!shm) return 1;

    auto* bridge = shm->get<SharedBridge>();
    while (keep_running && !bridge->mw_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!keep_running) return 1;

    const size_t total_slaves = bridge->topology_size.load();
    size_t total_motor_count = 0;
    size_t total_gripper_count = 0;

    std::unordered_set<int> valid_motor_ids;
    std::unordered_set<int> valid_gripper_ids;
    std::unordered_map<int, std::string> yaml_motor_names;
    std::unordered_map<int, std::string> yaml_gripper_names;

    for (const auto& m : cfg->motors) {
        valid_motor_ids.insert(m.ecat_id);
        yaml_motor_names[m.ecat_id] = m.name;
    }
    for (const auto& g : cfg->grippers) {
        valid_gripper_ids.insert(g.ecat_id);
        yaml_gripper_names[g.ecat_id] = g.name;
    }

    cfg->motors.clear();
    cfg->grippers.clear();
    cfg->joints.clear();

    std::vector<int> motor_topology_ids;
    std::vector<int> gripper_topology_ids;

    for (size_t i = 0; i < total_slaves; ++i) {
        const auto& slave = bridge->topology[i];
        const int id = static_cast<int>(slave.board_id);

        if (slave.type == DeviceType::MOTOR) {
            total_motor_count++;
            motor_topology_ids.push_back(id);
            
            JointConfig m_cfg;
            m_cfg.ecat_id = id;
            m_cfg.name = "motor_" + std::to_string(id);
            cfg->motors.push_back(m_cfg);

            // Filter for joint_states: ONLY if it matches the configuration file
            if (valid_motor_ids.count(id)) {
                JointConfig j_cfg;
                j_cfg.ecat_id = id;
                j_cfg.name = yaml_motor_names[id];
                cfg->joints.push_back(j_cfg);
            } else {
                std::cout << "[SHM-DDS Bridge] Motor ID " << id 
                          << " unconfigured. Omitted from joint_states.\n";
            }
        } 
        else if (slave.type == DeviceType::GRIPPER) {
            total_gripper_count++;
            gripper_topology_ids.push_back(id);

            JointConfig g_cfg;
            g_cfg.ecat_id = id;
            g_cfg.name = "gripper_" + std::to_string(id);
            cfg->grippers.push_back(g_cfg);

            if (valid_gripper_ids.count(id)) {
                JointConfig j_cfg;
                j_cfg.ecat_id = id;
                j_cfg.name = yaml_gripper_names[id];
                cfg->joints.push_back(j_cfg);
            } else {
                std::cout << "[SHM-DDS Bridge] Gripper ID " << id
                        << " unconfigured. Omitted from joint_states.\n";
            }
        }
    }

    std::cout << "[SHM-DDS Bridge] Configuration mapping breakdown:\n"
              << "  - Total hardware batch size (Motors): " << total_motor_count << "\n"
              << "  - Total hardware batch size (Grippers): " << total_gripper_count << "\n"
              << "  - Filtered joints for joint_state message: " << cfg->joints.size() << "\n";

    DdsAdapter dds_adapter;
    if (!dds_adapter.init(*cfg)) {
        std::cerr << "[SHM-DDS Bridge] Failed to bind to target DDS channels." << std::endl;
        return 1;
    }
    
    bridge->rt_ready.store(true);

    ShmProtoHelper proto_helper;
    ProtoSlot frame;
    size_t motors_in_batch = 0;
    size_t grippers_in_batch = 0;
    size_t joints_in_batch = 0;             
    const size_t total_joint_frames = total_motor_count + total_gripper_count;

    joint_state::rt_joint_state_msg joint_msg;
    motor::rt_motor_msg motor_msg;
    gripper::rt_gripper_msg gripper_msg;

    while (keep_running && bridge->mw_ready.load()) {
        bool active = false;

        if (proto_helper.pop_latest_frame(bridge->imu, frame)) {
            imu::rt_imu_msg imu_msg;
            if (pdo_utils::parse_imu_frame(frame.data, static_cast<ssize_t>(frame.size), imu_msg))
                dds_adapter.publish(imu_msg);
            active = true;
        }

        while (bridge->motor.try_pop(frame)) {
            active = true;
            if (motors_in_batch == 0) motor_msg = motor::rt_motor_msg{};
            if (joints_in_batch == 0) joint_msg = joint_state::rt_joint_state_msg{};

            if (motors_in_batch < motor_topology_ids.size()) {
                const bool include = valid_motor_ids.count(motor_topology_ids[motors_in_batch]) > 0;
                pdo_utils::parse_joint_frame(frame.data, static_cast<ssize_t>(frame.size),
                                            joint_msg, motor_msg, include);
            }
            
            ++joints_in_batch;
            ++motors_in_batch;
            
            if (motors_in_batch == total_motor_count) {
                dds_adapter.publish(motor_msg);
                motors_in_batch = 0;
            }
        }

        while (bridge->gripper.try_pop(frame)) {
            active = true;
            if (grippers_in_batch == 0) gripper_msg = gripper::rt_gripper_msg{};
            if (joints_in_batch == 0) joint_msg = joint_state::rt_joint_state_msg{};

            if (grippers_in_batch < gripper_topology_ids.size()) {
                const bool include = valid_gripper_ids.count(gripper_topology_ids[grippers_in_batch]) > 0;
                pdo_utils::parse_gripper_frame(frame.data, static_cast<ssize_t>(frame.size),
                                                gripper_msg, joint_msg, include);
            }
            
            ++joints_in_batch;
            ++grippers_in_batch;
            
            if (grippers_in_batch == total_gripper_count) {
                dds_adapter.publish(gripper_msg);
                grippers_in_batch = 0;
            }
        }

        if (joints_in_batch >= total_joint_frames) {
            dds_adapter.publish(joint_msg);
            joints_in_batch = 0; 
        }

        if (!active)
            std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "[SHM-DDS Bridge] Disconnected from shared memory pipeline. Shutting down." << std::endl;
    bridge->rt_ready.store(false);

    return 0;
}
