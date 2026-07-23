#include <chrono>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

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

    try
    {
        cdr.read_encapsulation();

        int32_t stamp_sec;
        uint32_t stamp_nanosec;
        std::string frame_id;
        cdr >> stamp_sec;
        cdr >> stamp_nanosec;
        cdr >> frame_id;

        double qx, qy, qz, qw;
        cdr >> qx >> qy >> qz >> qw;

        double wx, wy, wz;
        cdr >> wx >> wy >> wz;

        double ax, ay, az;
        cdr >> ax >> ay >> az;

        std::cout << "IMU [" << frame_id << "] stamp=" << stamp_sec << "."
                   << stamp_nanosec
                   << " quat=(" << qx << ", " << qy << ", " << qz << ", " << qw << ")"
                   << " gyro=(" << wx << ", " << wy << ", " << wz << ")"
                   << " accel=(" << ax << ", " << ay << ", " << az << ")\n";
    }
    catch (const eprosima::fastcdr::exception::Exception& e)
    {
        std::cerr << "<failed to decode IMU sample: " << e.what() << ">\n";
    }
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