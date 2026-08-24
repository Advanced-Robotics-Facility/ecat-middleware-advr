#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <advrf_interfaces/msg/Imu.hpp>
#include <advrf_interfaces/msg/ImuCdrAux.hpp>

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

void decode_and_print_imu(const zenoh::Bytes& payload)
{
    auto bytes = payload.as_vector();
    eprosima::fastcdr::FastBuffer buffer(
        reinterpret_cast<char*>(bytes.data()), bytes.size());
    eprosima::fastcdr::Cdr cdr(buffer);
    cdr.read_encapsulation();

    advrf_interfaces::msg::dds_::Imu_ imu;
    eprosima::fastcdr::deserialize(cdr, imu);

    std::cout << "IMU [" << imu.header().frame_id() << "] stamp="
              << imu.header().stamp().sec() << "."
              << imu.header().stamp().nanosec()
              << " accel=(" << imu.linear_acceleration().x() << ", "
              << imu.linear_acceleration().y() << ", "
              << imu.linear_acceleration().z() << ")"
              << " gyro=(" << imu.angular_velocity().x() << ", "
              << imu.angular_velocity().y() << ", "
              << imu.angular_velocity().z() << ")"
              << " quat=(" << imu.orientation().x() << ", "
              << imu.orientation().y() << ", "
              << imu.orientation().z() << ", "
              << imu.orientation().w() << ")\n";
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
