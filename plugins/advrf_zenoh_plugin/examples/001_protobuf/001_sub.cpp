#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace {
    volatile std::sig_atomic_t running = 1;
    void stop(int) { running = 0; }

    void decode_and_print_imu(const zenoh::Bytes& payload)
    {
        auto bytes = payload.as_vector();

        iit::advrf::Ec_slave_pdo pdo;
        if (!pdo.ParseFromArray(bytes.data(), static_cast<int>(bytes.size()))) {
            std::cerr << "<failed to parse IMU pdo>\n";
            return;
        }

        const auto& hdr = pdo.header();
        const auto& imu = pdo.imuvn_rx_pdo();

        std::cout << "IMU [" << hdr.str_id() << "] idx=" << hdr.index()
                << " stamp=" << hdr.stamp().sec() << "." << hdr.stamp().nsec()
                << " gyro=(" << imu.x_rate() << ", " << imu.y_rate() << ", " << imu.z_rate() << ")"
                << " accel=(" << imu.x_acc() << ", " << imu.y_acc() << ", " << imu.z_acc() << ")"
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