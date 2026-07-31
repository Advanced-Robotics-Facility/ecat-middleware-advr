#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <csignal>
#include <string>

#include "advrf_middleware_core/config/robot_config.hpp"

#include <ecat_master_future/shm/config.hpp>
#include <ecat_master_future/shm/bridge_struct.hpp>
#include <ecat_master_future/shm/shared_memory.hpp>
#include <ecat_master_future/shm/shared_types.hpp>
#include <ecat_master_future/shm/proto_helper.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

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

iit::advrf::Ec_slave_pdo make_motor_pdo(double t, uint64_t sample_index, int motor_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_CIA402);
    populate_pdo_header(pdo, "motor_" + std::to_string(motor_id), sample_index);

    const double phase = t + 0.2 * motor_id;

    auto* payload = pdo.mutable_cia402_rx_pdo();

    payload->set_statusword(0x1234);
    payload->set_modes_of_op(8);

    payload->set_motor_pos(static_cast<float>(std::sin(phase)));
    payload->set_motor_vel(static_cast<float>(std::cos(phase)));
    payload->set_link_pos(static_cast<float>(std::sin(phase)));
    payload->set_link_vel(static_cast<float>(std::cos(phase)));
    payload->set_current(static_cast<float>(0.0));
    payload->set_torque(static_cast<float>(0.0));
    payload->set_demanded_pos(static_cast<float>(0.0));
    payload->set_demanded_vel(static_cast<float>(0.0));
    payload->set_demanded_current(static_cast<float>(0.0));
    payload->set_demanded_torque(static_cast<float>(0.0));
    payload->set_control_effort(static_cast<float>(0.0));
    payload->set_motor_temp(static_cast<float>(0.0));
    payload->set_drive_temp(35.5);
    payload->set_error_code(0);
    payload->set_error_report("");

    return pdo;
}

iit::advrf::Ec_slave_pdo make_gripper_pdo(double t, uint64_t sample_index, int gripper_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_GRIPPER);
    populate_pdo_header(pdo, "gripper_" + std::to_string(gripper_id), sample_index);

    const double phase = t + 0.2 * gripper_id;

    auto* payload = pdo.mutable_gripper_rx_pdo();

    payload->set_statusword(0x4321);
    payload->set_motor_pos(static_cast<float>(std::sin(phase)));
    payload->set_link_pos(static_cast<float>(std::sin(phase)));
    payload->set_demanded_pos(static_cast<float>(0.0));
    payload->set_demanded_vel(static_cast<float>(0.0));
    payload->set_error_code(0);

    return pdo;
}

iit::advrf::Ec_slave_pdo make_valve_pdo(double t, uint64_t sample_index, int valve_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_HYQ_KNEE);
    populate_pdo_header(pdo, "valve_" + std::to_string(valve_id), sample_index);

    const double phase = t + 0.2 * valve_id;

    auto* payload = pdo.mutable_hyqknee_rx_pdo();

    payload->set_encoder_position(static_cast<float>(std::sin(phase)));
    payload->set_force(static_cast<float>(std::cos(phase)));
    payload->set_pressure_1(static_cast<float>(std::sin(phase)));
    payload->set_pressure_2(static_cast<float>(std::cos(phase)));
    payload->set_current(static_cast<float>(0.0));
    payload->set_temperature(static_cast<float>(0.0));
    payload->set_fault(0);
    payload->set_rtt(0);
    payload->set_op_idx_ack(0);
    payload->set_aux(static_cast<float>(0.0));
    payload->set_current_ref_fb(static_cast<float>(0.0));
    payload->set_position_ref_fb(static_cast<float>(0.0));
    payload->set_force_ref_fb(static_cast<float>(0.0));

    return pdo;
}

iit::advrf::Ec_slave_pdo make_imu_pdo(double t, uint64_t sample_index, int imu_id)
{
    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_IMU_VN);
    populate_pdo_header(pdo, "imu_" + std::to_string(imu_id), sample_index);

    auto* payload = pdo.mutable_imuvn_rx_pdo();

    payload->set_x_rate(static_cast<float>(0.10 * std::sin(t)));
    payload->set_y_rate(static_cast<float>(0.20 * std::cos(t)));
    payload->set_z_rate(static_cast<float>(0.05 * std::sin(t * 0.5)));

    payload->set_x_acc(static_cast<float>(0.15 * std::sin(t * 0.7)));
    payload->set_y_acc(static_cast<float>(0.10 * std::cos(t * 0.4)));
    payload->set_z_acc(static_cast<float>(9.81 + 0.5 * std::sin(t)));

    const double yaw = 0.25 * std::sin(t * 0.2);
    payload->set_x_quat(0.0f);
    payload->set_y_quat(0.0f);
    payload->set_z_quat(static_cast<float>(std::sin(yaw * 0.5)));
    payload->set_w_quat(static_cast<float>(std::cos(yaw * 0.5)));

    payload->set_temperature(static_cast<uint32_t>(35 + (sample_index % 4)));
    payload->set_imu_ts(static_cast<uint32_t>(sample_index));
    payload->set_digital_in(static_cast<uint32_t>(sample_index & 0x1));
    payload->set_fault(0);
    payload->set_rtt(2);

    return pdo;
}

iit::advrf::Ec_slave_pdo make_pb_pdo(double t, uint64_t sample_index, int power_board_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_POW_F28M36);
    populate_pdo_header(pdo, "power_board_" + std::to_string(power_board_id), sample_index);

    const double phase = t + 0.2 * power_board_id;

    auto* payload = pdo.mutable_powf28m36_rx_pdo();

    payload->set_v_batt(static_cast<float>(std::sin(phase)));
    payload->set_v_load(static_cast<float>(std::sin(phase)));
    payload->set_i_load(static_cast<float>(std::cos(phase)));
    payload->set_temp_pcb(static_cast<float>(0.0));
    payload->set_temp_heatsink(static_cast<float>(0.0));
    payload->set_temp_batt(static_cast<float>(0.0));
    payload->set_status(1);
    payload->set_fault(0);
    payload->set_rtt(static_cast<float>(0.0));
    payload->set_op_idx_ack(0);
    payload->set_aux(static_cast<float>(0.0));

    return pdo;
}

iit::advrf::Ec_slave_pdo make_pump_pdo(double t, uint64_t sample_index, int pump_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_HYQ_HPU);
    populate_pdo_header(pdo, "pump_" + std::to_string(pump_id), sample_index);

    const double phase = t + 0.2 * pump_id;

    auto* payload = pdo.mutable_hyqhpu_rx_pdo();

    payload->set_motor_current(static_cast<float>(std::sin(phase)));
    payload->set_motor_speed(static_cast<float>(std::sin(phase)));
    payload->set_pressure1(static_cast<float>(std::cos(phase)));
    payload->set_pressure2(static_cast<float>(std::cos(phase)));
    payload->set_temperature(40);
    payload->set_mosfet_temperature(0);
    payload->set_motor_temperature(0);
    payload->set_fault(0);
    payload->set_rtt(0);
    payload->set_op_idx_ack(0);
    payload->set_aux(static_cast<float>(0.0));

    return pdo;
}

iit::advrf::Ec_slave_pdo make_ft_pdo(double t, uint64_t sample_index, int force_torque_id) {

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_FT6);
    populate_pdo_header(pdo, "force_torque_" + std::to_string(force_torque_id), sample_index);

    const double phase = t + 0.2 * force_torque_id;

    auto* payload = pdo.mutable_ft6_rx_pdo();

    payload->set_force_x(static_cast<float>(std::sin(phase)));
    payload->set_force_y(static_cast<float>(std::sin(phase)));
    payload->set_force_z(static_cast<float>(std::cos(phase)));
    payload->set_torque_x(static_cast<float>(std::cos(phase)));
    payload->set_torque_y(static_cast<float>(std::sin(phase)));
    payload->set_torque_z(static_cast<float>(std::sin(phase)));
    payload->set_fault(0);
    payload->set_rtt(0);
    payload->set_op_idx_ack(0);
    payload->set_aux(static_cast<float>(0.0));

    return pdo;
}
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;

    // Publisher SHM
    auto pub_shm = SharedMemory<SharedPubBridge>::create(SHM_PUB_NAME);

    // REPL SHM
    auto repl_shm = SharedMemory<SharedReplBridge>::create(SHM_REPL_NAME);

    // SUB SHM
    auto sub_shm = SharedMemory<SharedSubBridge>::create(SHM_SUB_NAME);

    // Dynamic Discovery Generation Loop
    uint32_t slave_idx = 0;
    
    auto add_devices = [&](const auto& devices, DeviceType type, const char* type_name) {
        for (const auto& dev : devices) {
            if (slave_idx >= MAX_SLAVES_CAPACITY) {
                std::cerr << "[Producer] MAX_SLAVES_CAPACITY reached, dropping '" << type_name
                        << " '" << dev.name
                        << "' (ecat_id=" << dev.ecat_id << ")\n";
                break;
            }
            if (dev.ecat_id < 0) {
                std::cerr << "[Producer] Skipping '" << type_name
                          << " '" << dev.name 
                          << "' with invalid ecat_id " << dev.ecat_id << '\n';
                continue;
            }
            auto& slave = pub_shm->bridge().payload.topology[slave_idx++];
            slave.board_id = static_cast<uint32_t>(dev.ecat_id);
            slave.type = type;
            std::snprintf(slave.name, sizeof(slave.name), "%s", dev.name.c_str());     
        }
    };

    add_devices(cfg->imus, DeviceType::IMU, "imu");
    add_devices(cfg->motors, DeviceType::MOTOR, "motor");
    add_devices(cfg->grippers, DeviceType::GRIPPER, "gripper");
    add_devices(cfg->power_boards, DeviceType::POWER_BOARD, "power_board");
    add_devices(cfg->pumps, DeviceType::PUMP, "pump");
    add_devices(cfg->force_torques, DeviceType::FORCE_TORQUE, "force_torque");
    add_devices(cfg->valves, DeviceType::VALVE, "valve");

    pub_shm->bridge().payload.topology_size.store(slave_idx);
    pub_shm->bridge().status.rt_ready.store(true);
    sub_shm->bridge().status.rt_ready.store(true);
    repl_shm->bridge().status.rt_ready.store(true);

    std::cout << "\n=======================================\n";
    std::cout << "[Producer] Bus Discovery Finished. Total Slaves Registered: " << slave_idx << "\n";
    std::cout << "-----------------------------------------\n";
    std::cout << " Board ID | Shared Memory Identifier \n";
    std::cout << "-----------------------------------------\n";
    
    for (uint32_t i = 0; i < slave_idx; ++i) {
        const auto& slave = pub_shm->bridge().payload.topology[i];

        std::printf("    %2u    | %s\n", 
                     slave.board_id, slave.name);
    }
    std::cout << "=========================================\n\n";

    ShmProtoHelper repl_proto_helper;
    iit::advrf::Repl_cmd cmd_msg;
     
    ShmProtoHelper proto_helper;
    double t = 0.0;
    uint64_t sample_count = 0;
    bool bridge_seen = false;

    auto next_tick = std::chrono::steady_clock::now();
    while (keep_running) {
        
        repl_proto_helper.drain(repl_shm->bridge().payload.request, cmd_msg, [&](const iit::advrf::Repl_cmd& cmd) {
      
            iit::advrf::Cmd_reply reply;
            reply.mutable_request_id()->CopyFrom(cmd.request_id());
            reply.set_type(iit::advrf::Cmd_reply::ACK);
            reply.set_cmd_type(cmd.type());
            reply.set_msg("test ack received");

            if (cmd.type() == iit::advrf::CmdType::ECAT_MASTER_CMD) {
                const auto& ecat_cmd = cmd.ecat_master_cmd();

                switch (ecat_cmd.type()) {
                    case iit::advrf::Ecat_Master_cmd::GET_SLAVES_DESCR: {
                        const uint32_t n = pub_shm->bridge().payload.topology_size.load();

                        std::ostringstream oss;
                        oss << n << " slaves: ";
                        for (uint32_t i = 0; i < n; ++i) {
                            const auto& slave = pub_shm->bridge().payload.topology[i];
                            oss << slave.name << "(id=" << slave.board_id << ") ";
                        }

                        reply.set_msg(oss.str());
                        break;
                    }

                    case iit::advrf::Ecat_Master_cmd::START_MASTER:
                        reply.set_msg("mock: master already running (no-op)");
                        break;

                    case iit::advrf::Ecat_Master_cmd::STOP_MASTER:
                        reply.set_msg("mock: stop not implemented in mock master");
                        break;

                    default:
                        reply.set_msg("mock: unhandled Ecat_Master_cmd type");
                        break;
                }
            }

            if (!repl_proto_helper.push(repl_shm->bridge().payload.reply, reply)) {
                std::cerr << "[Producer] Failed to push repl reply (queue full)" << '\n';
            }
        });

        for (uint32_t i = 0; i < slave_idx; ++i) {
            const auto& slave = pub_shm->bridge().payload.topology[i];

            switch (slave.type) {
                case DeviceType::IMU:
                    proto_helper.push(pub_shm->bridge().payload.imu, make_imu_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::MOTOR:
                    proto_helper.push(pub_shm->bridge().payload.motor, make_motor_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::GRIPPER:
                    proto_helper.push(pub_shm->bridge().payload.gripper, make_gripper_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::POWER_BOARD:
                proto_helper.push(pub_shm->bridge().payload.power_board, make_pb_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::PUMP:
                    proto_helper.push(pub_shm->bridge().payload.pump, make_pump_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::FORCE_TORQUE:
                    proto_helper.push(pub_shm->bridge().payload.force_torque, make_ft_pdo(t, sample_count, slave.board_id));
                    break;
                case DeviceType::VALVE:
                    proto_helper.push(pub_shm->bridge().payload.valve, make_valve_pdo(t, sample_count, slave.board_id));
                    break;
                default:
                    break;
            }
        }

        ++sample_count;
        t += 0.001;

        if (!bridge_seen && pub_shm->bridge().status.rt_ready.load()) {
            bridge_seen = true;
            std::cout << "[Producer] DDS bridge connected." << '\n';
        }

        next_tick += std::chrono::milliseconds(1);
        std::this_thread::sleep_until(next_tick);
    }

    pub_shm->bridge().status.mw_ready.store(false);
    pub_shm->bridge().status.rt_ready.store(false);

    return 0;
}