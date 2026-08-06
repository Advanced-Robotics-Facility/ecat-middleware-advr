#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zenoh.hxx>

#include <ecat_master_future/shm/config.hpp>
#include <ecat_master_future/shm/shared_memory.hpp>
#include <ecat_master_future/shm/proto_helper.hpp>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace {
    volatile std::sig_atomic_t running = 1;
    void stop(int) { running = 0; }
} 

int main()
{
    using namespace std::chrono_literals;

    constexpr const char* key = "rt/advrf/kyon/imu/imu_1";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    // Open SHM
    auto shm = SharedMemory<SharedPubBridge>::open(SHM_PUB_NAME);
    if (!shm) {
        std::cerr << "Failed to open shared memory: " << SHM_PUB_NAME << '\n';
        return 1;
    }

    auto& bridge = shm->bridge();

    while (running && !bridge.status.mw_ready.load(std::memory_order_relaxed)) {
        if (!shm->owner_alive()) {
            std::cerr << "Shared-memory owner is not alive\n";
            return 1;
        }

        std::this_thread::sleep_for(50ms);
    }

    if (!running) return 0;

    try
    {
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto publisher = session.declare_publisher(zenoh::KeyExpr(key));

        ShmProtoHelper proto_helper;

        while (running) {
            proto_helper.drain_raw(bridge.payload.imu, [&](const uint8_t* data, size_t size) {
                std::vector<uint8_t> payload(data, data + size);

                publisher.put(zenoh::Bytes(std::move(payload)));
                std::cout << "Published raw IMU protobuf, size=" << size << " bytes\n";
            });

            std::this_thread::sleep_for(1ms);
        }
    }
    catch (const zenoh::ZException& error)
    {
        std::cerr << "Zenoh IMU bridge error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}