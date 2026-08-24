#include <cstdint>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include <ecat_master_future/shm/config.hpp>
#include <ecat_master_future/shm/shared_memory.hpp>
#include <ecat_master_future/shm/proto_helper.hpp>
#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

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
    auto shm = SharedMemory<SharedPubBridge>::open(SHM_PUB_NAME);
    if (!shm) {
        std::cerr << "Failed to open shared memory: " << SHM_PUB_NAME << '\n';
        return 1;
    }

    auto& bridge = shm->bridge();
    while (running && !bridge.status.rt_ready.load()) {
        if (!shm->owner_alive()) {
            std::cerr << "Shared-memory owner is not alive\n";
            return 1;
        }

        std::this_thread::sleep_for(50ms);
    }

    if (!running) return 0;

    try
    {
        /* Note:
            I you want to change endpoint:
                auto config = zenoh::Config::create_default();
                config.insert_json5(
                    "connect/endpoints",
                    "[\"tcp/localhost:7447\"]"
                );
                auto session = zenoh::Session::open(std::move(config));
        */
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto publisher = session.declare_publisher(zenoh::KeyExpr(key));

        ShmProtoHelper proto_helper;
        iit::advrf::Ec_slave_pdo pdo;
        const advrf::zenoh_plugin::serialization::Ros2CdrSerializer serializer(
            advrf::zenoh_plugin::serialization::Ros2MessageType::Imu);

        std::vector<std::uint8_t> payload;

        while (running) {
            proto_helper.drain(bridge.payload.imu, pdo, [&](const iit::advrf::Ec_slave_pdo& msg) {
                if (!serializer.serialize_cycle({msg}, payload)) {
                    std::cerr << "Failed to serialize Imu PDO as ROS2 CDR.\n";
                    return;
                }
                publisher.put(zenoh::Bytes(std::move(payload)));
                std::cout << "Published CDR IMU [" << msg.header().str_id()
                          << "] stamp=" << msg.header().stamp().sec()
                          << "." << msg.header().stamp().nsec() << '\n';
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
