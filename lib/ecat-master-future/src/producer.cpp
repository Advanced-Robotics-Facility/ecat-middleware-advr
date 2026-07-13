#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <csignal>
#include <new>
#include <string>
#include <vector>

#include "shm_utils.hpp"
#include "shm_shared_types.hpp"
#include "robot_config.hpp"
#include "ecat_pdo.pb.h"

namespace {
volatile std::sig_atomic_t keep_running = 1;

void on_signal(int)
{
    keep_running = 0;
}

uint64_t monotonic_now_ns()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

void populate_pdo_header(iit::advrf::Ec_slave_pdo& pdo, const std::string& id, uint64_t sample_index) {
    auto* header = pdo.mutable_header();
    header->set_str_id(id);
    header->set_index(static_cast<int32_t>(sample_index));

    const auto now_ns = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
    auto* stamp = header->mutable_stamp();
    stamp->set_sec(static_cast<int32_t>(now_ns / 1'000'000'000ULL));
    stamp->set_nsec(static_cast<int32_t>(now_ns % 1'000'000'000ULL));
}

iit::advrf::Ec_slave_pdo make_imu_pdo(double t, uint64_t sample_index, int imu_id)
{
    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_IMU_VN);
    populate_pdo_header(pdo, "imu" + std::to_string(imu_id), sample_index);

    auto* imu_payload = pdo.mutable_imuvn_rx_pdo();

    imu_payload->set_x_rate(static_cast<float>(0.10 * std::sin(t)));
    imu_payload->set_y_rate(static_cast<float>(0.20 * std::cos(t)));
    imu_payload->set_z_rate(static_cast<float>(0.05 * std::sin(t * 0.5)));

    imu_payload->set_x_acc(static_cast<float>(0.15 * std::sin(t * 0.7)));
    imu_payload->set_y_acc(static_cast<float>(0.10 * std::cos(t * 0.4)));
    imu_payload->set_z_acc(static_cast<float>(9.81 + 0.5 * std::sin(t)));

    const double yaw = 0.25 * std::sin(t * 0.2);
    imu_payload->set_x_quat(0.0f);
    imu_payload->set_y_quat(0.0f);
    imu_payload->set_z_quat(static_cast<float>(std::sin(yaw * 0.5)));
    imu_payload->set_w_quat(static_cast<float>(std::cos(yaw * 0.5)));

    imu_payload->set_temperature(static_cast<uint32_t>(35 + (sample_index % 4)));
    imu_payload->set_imu_ts(static_cast<uint32_t>(sample_index));
    imu_payload->set_digital_in(static_cast<uint32_t>(sample_index & 0x1));
    imu_payload->set_fault(0);
    imu_payload->set_rtt(2);

    return pdo;
}

iit::advrf::Ec_slave_pdo make_motor_pdo(double t, uint64_t sample_index, int motor_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_CIA402);
    populate_pdo_header(pdo, "motor_" + std::to_string(motor_id), sample_index);

    const double phase = t + 0.2 * motor_id;

    auto* motor_payload = pdo.mutable_cia402_rx_pdo();

    motor_payload->set_statusword(0x1234);
    motor_payload->set_modes_of_op(8);

    motor_payload->set_motor_pos(static_cast<float>(std::sin(phase)));
    motor_payload->set_motor_vel(static_cast<float>(std::cos(phase)));
    motor_payload->set_link_pos(static_cast<float>(std::sin(phase)));
    motor_payload->set_link_vel(static_cast<float>(std::cos(phase)));
    motor_payload->set_current(static_cast<float>(0.0));
    motor_payload->set_torque(static_cast<float>(0.0));
    motor_payload->set_demanded_pos(static_cast<float>(0.0));
    motor_payload->set_demanded_vel(static_cast<float>(0.0));
    motor_payload->set_demanded_current(static_cast<float>(0.0));
    motor_payload->set_demanded_torque(static_cast<float>(0.0));
    motor_payload->set_control_effort(static_cast<float>(0.0));
    motor_payload->set_motor_temp(static_cast<float>(0.0));
    motor_payload->set_drive_temp(35.5);
    motor_payload->set_error_code(0);
    motor_payload->set_error_report("");

    return pdo;
}

iit::advrf::Ec_slave_pdo make_gripper_pdo(double t, uint64_t sample_index, int gripper_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_GRIPPER);
    populate_pdo_header(pdo, "gripper_" + std::to_string(gripper_id), sample_index);

    const double phase = t + 0.2 * gripper_id;

    auto* gripper_payload = pdo.mutable_gripper_rx_pdo();

    gripper_payload->set_statusword(0x4321);
    gripper_payload->set_motor_pos(static_cast<float>(std::sin(phase)));
    gripper_payload->set_link_pos(static_cast<float>(std::sin(phase)));
    gripper_payload->set_demanded_pos(static_cast<float>(0.0));
    gripper_payload->set_demanded_vel(static_cast<float>(0.0));
    gripper_payload->set_error_code(0);

    return pdo;
}
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;

    std::cout << "[Producer] Initializing shared memory segment: " << SHM_NAME << std::endl;

    SharedMemoryOwner shm(SHM_NAME, sizeof(SharedBridge));
    if (!shm.is_valid()) {
        std::cerr << "Failed to allocate or map shared memory segment." << std::endl;
        return 1;
    }

    auto* bridge = shm.get<SharedBridge>();
    new (bridge) SharedBridge{};

    // Dynamic Discovery Generation Loop
    uint32_t slave_idx = 0;
    
    // IMUs
    const size_t imu_count = 1; 
    for (size_t i = 1; i <= imu_count && slave_idx < MAX_SLAVES_CAPACITY; ++i) {
        auto& slave = bridge->topology[slave_idx++];
        slave.board_id = slave_idx;
        slave.type = DeviceType::IMU;
        std::snprintf(slave.name, sizeof(slave.name), "imu_%zu", i);
    }

    // Motors
    const size_t motor_count = 12; 
    for (size_t i = 1; i <= motor_count && slave_idx < MAX_SLAVES_CAPACITY; ++i) {
        auto& slave = bridge->topology[slave_idx++];
        slave.board_id = slave_idx;
        slave.type = DeviceType::MOTOR;
        std::snprintf(slave.name, sizeof(slave.name), "motor_%ld", i);
    }

    // Grippers
    const size_t gripper_count = 2; 
    for (size_t i = 1; i <= gripper_count && slave_idx < MAX_SLAVES_CAPACITY; ++i) {
        auto& slave = bridge->topology[slave_idx++];
        slave.board_id = slave_idx;
        slave.type = DeviceType::GRIPPER;
        std::snprintf(slave.name, sizeof(slave.name), "gripper_%ld", i);
    }

    bridge->topology_size.store(slave_idx);
    bridge->mw_ready.store(true);
    bridge->rt_ready.store(false);

    std::cout << "\n=======================================\n";
    std::cout << "[Producer] Bus Discovery Finished. Total Slaves Registered: " << slave_idx << "\n";
    std::cout << "-----------------------------------------\n";
    std::cout << " Board ID | Shared Memory Identifier \n";
    std::cout << "-----------------------------------------\n";
    
    for (uint32_t i = 0; i < slave_idx; ++i) {
        const auto& slave = bridge->topology[i];

        std::printf("    %2u    | %s\n", 
                     slave.board_id, slave.name);
    }
    std::cout << "=========================================\n\n";

    ShmProtoHelper proto_helper;
    double t = 0.0;
    uint64_t sample_count = 0;
    bool bridge_seen = false;

    auto next_tick = std::chrono::steady_clock::now();
    while (keep_running) {
        
        for (uint32_t i = 0; i < slave_idx; ++i) {
            const auto& slave = bridge->topology[i];

            if (slave.type == DeviceType::IMU) {
                proto_helper.push(bridge->imu, make_imu_pdo(t, sample_count, slave.board_id));
            } 
            else if (slave.type == DeviceType::MOTOR) {
                proto_helper.push(bridge->motor, make_motor_pdo(t, sample_count, slave.board_id));
            } 
            else if (slave.type == DeviceType::GRIPPER) {
                proto_helper.push(bridge->gripper, make_gripper_pdo(t, sample_count, slave.board_id));
            }
        }

        ++sample_count;
        t += 0.001;

        if (!bridge_seen && bridge->rt_ready.load()) {
            bridge_seen = true;
            std::cout << "[Producer] DDS bridge connected." << std::endl;
        }

        next_tick += std::chrono::milliseconds(1);
        std::this_thread::sleep_until(next_tick);
    }

    bridge->mw_ready.store(false);
    bridge->rt_ready.store(false);

    return 0;
}