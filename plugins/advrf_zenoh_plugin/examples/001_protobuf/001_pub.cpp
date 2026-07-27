#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include "ecat_master_future/shm_utils.hpp"
#include "ecat_master_future/shm_shared_types.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace {
    volatile std::sig_atomic_t running = 1;
    void stop(int) { running = 0; }
} 

int main()
{
    using namespace std::chrono_literals;

    const std::string key = "advrf/robot/imu";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    // Open SHM
    SharedMemoryClient shm(SHM_NAME, sizeof(SharedPubBridge));
    if (!shm.is_valid()) {
        std::cerr << "Failed to attach to shared memory segment. Is the producer running?\n";
        return 1;
    }
    auto* bridge = shm.get<SharedPubBridge>();

    while (running && !bridge->mw_ready.load()) {
        std::this_thread::sleep_for(50ms);
    }
    if (!running) return 0;

    try
    {
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto publisher = session.declare_publisher(zenoh::KeyExpr(key));

        ShmProtoHelper proto_helper;

        while (running) {
            proto_helper.drain_raw(bridge->imu, [&](const uint8_t* data, size_t size) {
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