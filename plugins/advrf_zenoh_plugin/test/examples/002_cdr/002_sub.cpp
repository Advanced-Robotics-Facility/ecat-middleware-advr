#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include "advrf_zenoh_plugin/serialization/ros2_cdr.hpp"

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

void decode_and_print_imu(const zenoh::Bytes& payload)
{
    iit::advrf::Header header;
    iit::advrf::ImuVN_rx_pdo imu;
    if (!advrf::zenoh_plugin::ros2_cdr::deserialize(
            payload, header, imu))
    {
        std::cerr << "<failed to decode IMU sample>\n";
        return;
    }

    std::cout << "IMU [" << header.str_id() << "] stamp="
              << header.stamp().sec() << "." << header.stamp().nsec()
              << " accel=(" << imu.x_acc() << ", " << imu.y_acc() << ", "
              << imu.z_acc() << ")"
              << " gyro=(" << imu.x_rate() << ", " << imu.y_rate() << ", "
              << imu.z_rate() << ")"
              << " quat=(" << imu.x_quat() << ", " << imu.y_quat() << ", "
              << imu.z_quat() << ", " << imu.w_quat() << ")\n";
}
}

int main()
{
    using namespace std::chrono_literals;

    const std::string key = "advrf/robot/imu";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try
    {
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto subscriber = session.declare_subscriber(
            zenoh::KeyExpr(key),
            [](const zenoh::Sample& sample)
            {
                decode_and_print_imu(sample.get_payload());
            },
            zenoh::closures::none);

        std::cout << "Subscribed to IMU topic '" << key << "'. Press Ctrl-C to stop.\n";
        while (running)
        {
            std::this_thread::sleep_for(200ms);
        }
    }
    catch (const zenoh::ZException& error)
    {
        std::cerr << "Zenoh IMU subscriber error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
