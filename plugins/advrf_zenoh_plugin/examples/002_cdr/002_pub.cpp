#include <atomic>
#include <chrono>
#include <cstdint>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zenoh.hxx>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include "ecat_master_future/shm_utils.hpp"
#include "ecat_master_future/shm_shared_types.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

struct ImuSample
{
    int32_t stamp_sec;
    uint32_t stamp_nanosec;
    std::string frame_id;
    double ax, ay, az;
    double wx, wy, wz;
    double qx, qy, qz, qw;
    uint32_t imu_ts;
    uint32_t temperature;
    uint32_t digital_in;
    uint32_t fault;
    uint32_t rtt; 
};

std::vector<uint8_t> cdr_imu(const ImuSample& s)
{
    eprosima::fastcdr::FastBuffer buffer;
    eprosima::fastcdr::Cdr cdr(buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                                eprosima::fastcdr::CdrVersion::XCDRv1);

    cdr.serialize_encapsulation();

    // std_msgs/Header
    cdr << s.stamp_sec;
    cdr << s.stamp_nanosec;
    cdr << s.frame_id;

    // geometry_msgs/Vector3 linear_acceleration
    cdr << s.ax << s.ay << s.az;

    // geometry_msgs/Vector3 angular_velocity
    cdr << s.wx << s.wy << s.wz;

    // geometry_msgs/Quaternion orientation
    cdr << s.qx << s.qy << s.qz << s.qw;

    cdr << s.imu_ts;
    cdr << s.temperature;
    cdr << s.digital_in;
    cdr << s.fault;
    cdr << s.rtt;

    const auto* data = reinterpret_cast<const uint8_t*>(buffer.getBuffer());
    return std::vector<uint8_t>(data, data + cdr.get_serialized_data_length());
}

ImuSample to_imu_sample(const iit::advrf::Ec_slave_pdo& pdo)
{
    const auto& hdr = pdo.header();
    const auto& imu = pdo.imuvn_rx_pdo();

    ImuSample s{};
    s.stamp_sec = hdr.stamp().sec();
    s.stamp_nanosec = static_cast<uint32_t>(hdr.stamp().nsec());
    s.frame_id = hdr.str_id();

    s.ax = imu.x_acc();
    s.ay = imu.y_acc();
    s.az = imu.z_acc();

    s.wx = imu.x_rate();
    s.wy = imu.y_rate();
    s.wz = imu.z_rate();

    s.qx = imu.x_quat();
    s.qy = imu.y_quat();
    s.qz = imu.z_quat();
    s.qw = imu.w_quat();

    s.imu_ts = imu.imu_ts();
    s.temperature = imu.temperature();
    s.digital_in = imu.digital_in();
    s.fault = imu.fault();
    s.rtt = imu.rtt();

    return s;
}
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

        while (running) {
            proto_helper.drain(bridge->imu, pdo, [&](const iit::advrf::Ec_slave_pdo& msg) {
                const ImuSample sample = to_imu_sample(msg);
                publisher.put(zenoh::Bytes(cdr_imu(sample)));
                std::cout << "Published IMU sample [" << sample.frame_id << "] stamp="
                           << sample.stamp_sec << "." << sample.stamp_nanosec << '\n';
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