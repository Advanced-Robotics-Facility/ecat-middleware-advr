#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <csignal>
#include <string>

#include "shm_utils.hpp"
#include "shm_shared_types.hpp"
#include "pdo_utils.hpp"
#include "publisher.hpp"

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

    const size_t motor_count = bridge->motor_count.load();
    cfg->motors.resize(motor_count);
    cfg->joints = cfg->motors;

    DdsAdapter dds_adapter;
    if (!dds_adapter.init(*cfg)) {
        std::cerr << "[SHM-DDS Bridge] Failed to bind to target DDS channels." << std::endl;
        return 1;
    }

    bridge->rt_ready.store(true);

    ShmProtoHelper proto_helper;
    ProtoSlot frame;
    size_t motors_in_batch = 0;
    joint_state::rt_joint_state_msg joint_msg;
    motor::rt_motor_msg motor_msg;

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

            if (motors_in_batch == 0) {
                joint_msg = joint_state::rt_joint_state_msg{};
                motor_msg = motor::rt_motor_msg{};
            }

            if (!pdo_utils::parse_joint_frame(frame.data, static_cast<ssize_t>(frame.size), joint_msg, motor_msg))
                continue;

            ++motors_in_batch;

            if (motors_in_batch == motor_count) {
                dds_adapter.publish(joint_msg);
                dds_adapter.publish(motor_msg);
                motors_in_batch = 0;
            }
        }

        if (!active)
            std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "[SHM-DDS Bridge] Disconnected from shared memory pipeline. Shutting down." << std::endl;
    bridge->rt_ready.store(false);

    return 0;
}
