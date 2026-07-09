#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <string>

#include "shm_utils.hpp" 
#include "shm_shared_types.hpp" 
#include <ecat-master-future/ecat_pdo.pb.h>
#include <ecat-master-future/pdo_utils.hpp>

#include "publisher.hpp"

namespace {
volatile std::sig_atomic_t keep_running = 1;

void on_signal(int)
{
    keep_running = 0;
}

uint32_t parse_domain_id(int argc, char** argv)
{
    const char* value = nullptr;
    if (argc > 2)
        value = argv[2];
    else
        value = std::getenv("ROS_DOMAIN_ID");

    if (value == nullptr)
        return 0;

    try {
        return static_cast<uint32_t>(std::stoul(value));
    } catch (const std::exception&) {
        std::cerr << "[SHM-DDS Bridge] Invalid domain id '" << value << "', using 0.\n";
        return 0;
    }
}

std::unique_ptr<SharedMemoryClient> wait_for_shared_memory()
{
    bool printed_wait = false;
    while (keep_running) {
        auto shm = std::make_unique<SharedMemoryClient>(SHM_NAME, sizeof(SharedBridge));
        if (shm->is_valid())
            return shm;

        if (!printed_wait)
            printed_wait = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return nullptr;
}

} 

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    const std::string robot_name = argc > 1 ? argv[1] : "test";
    const uint32_t domain_id = parse_domain_id(argc, argv);

    clock_utils::init();

    std::cout << "[SHM-DDS Bridge] Connecting to shared memory: " << SHM_NAME << std::endl;

    auto shm = wait_for_shared_memory();
    if (!shm)
        return 1;

    auto* bridge = shm->get<SharedBridge>();

    while (keep_running && !bridge->mw_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!keep_running)
        return 1;

    RobotConfig cfg;
    cfg.domain_id = domain_id;
    cfg.robot_name = robot_name;

    std::cout << "[SHM-DDS Bridge] DDS domain=" << cfg.domain_id
              << " ROS topic=/advrf/" << cfg.robot_name << "/imu" << std::endl;

    DdsAdapter dds_adapter;
    if (!dds_adapter.init(cfg)) {
        std::cerr << "[SHM-DDS Bridge] Failed to bind to target DDS channels." << std::endl;
        return 1;
    }

    bridge->rt_ready.store(true);

    ShmProtoHelper proto_helper;
    ProtoSlot frame;

    while (keep_running && bridge->mw_ready.load()) {
        if (proto_helper.pop_latest_frame(bridge->imu, frame)) {
            imu::rt_imu_msg imu_msg;
            if (pdo_utils::parse_imu_frame(frame.data, static_cast<ssize_t>(frame.size), imu_msg))
                dds_adapter.publish(imu_msg);
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    std::cout << "[SHM-DDS Bridge] Disconnected from shared memory pipeline. Shutting down." << std::endl;
    bridge->rt_ready.store(false);

    return 0;
}
