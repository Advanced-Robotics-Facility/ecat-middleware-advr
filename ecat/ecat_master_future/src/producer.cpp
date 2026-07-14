#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <csignal>
#include <new>

#include "ecat_master_future/shm_utils.hpp" 
#include "ecat_master_future/shm_shared_types.hpp" 
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

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

iit::advrf::Ec_slave_pdo make_imu_pdo(double t, uint64_t sample_index)
{
    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_IMU_VN);

    const uint64_t mono_ns = monotonic_now_ns();
    auto* header = pdo.mutable_header();
    header->set_str_id("imu");
    header->set_index(static_cast<int32_t>(sample_index));

    auto* stamp = header->mutable_stamp();
    stamp->set_sec(static_cast<int32_t>(mono_ns / 1'000'000'000ULL));
    stamp->set_nsec(static_cast<int32_t>(mono_ns % 1'000'000'000ULL));

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
} 

int main()
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    std::cout << "[IMU Producer] Initializing shared memory segment: " << SHM_NAME << std::endl;

    SharedMemoryOwner shm(SHM_NAME, sizeof(SharedBridge));
    if (!shm.is_valid()) {
        std::cerr << "Failed to allocate or map shared memory segment." << std::endl;
        return 1;
    }

    auto* bridge = shm.get<SharedBridge>();
    new (bridge) SharedBridge{};

    std::cout << "[IMU Producer] Created a fresh shared memory segment." << std::endl;
    bridge->mw_ready.store(true);
    bridge->rt_ready.store(false);

    ShmProtoHelper proto_helper;

    double t = 0.0;
    uint64_t sample_count = 0;
    bool bridge_seen = false;

    std::cout << "[IMU Producer] Starting transmission loop at 1kHz..." << std::endl;

    auto next_tick = std::chrono::steady_clock::now();
    while (keep_running) {
        const auto pdo = make_imu_pdo(t, sample_count);

        proto_helper.push(bridge->imu, pdo);

        ++sample_count;
        t += 0.001;

        if (!bridge_seen && bridge->rt_ready.load()) {
            bridge_seen = true;
            std::cout << "[IMU Producer] DDS bridge connected." << std::endl;
        }

        next_tick += std::chrono::milliseconds(1);
        std::this_thread::sleep_until(next_tick);
    }

    bridge->mw_ready.store(false);
    bridge->rt_ready.store(false);

    return 0;
}
