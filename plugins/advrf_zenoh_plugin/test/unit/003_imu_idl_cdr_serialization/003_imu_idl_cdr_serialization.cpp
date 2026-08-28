#include <cassert>
#include <cstdint>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <advrf_interfaces/msg/Imu.hpp>
#include <advrf_interfaces/msg/ImuCdrAux.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

int main()
{
    iit::advrf::Header header;
    header.set_str_id("imu_1");
    header.mutable_stamp()->set_sec(123);
    header.mutable_stamp()->set_nsec(456);

    iit::advrf::ImuVN_rx_pdo imu;
    imu.set_x_acc(1.25F);
    imu.set_y_rate(2.5F);
    imu.set_w_quat(0.75F);
    imu.set_imu_ts(10);
    imu.set_temperature(20);
    imu.set_digital_in(30);
    imu.set_fault(40);
    imu.set_rtt(50);

    iit::advrf::Ec_slave_pdo pdo;
    pdo.set_type(iit::advrf::Ec_slave_pdo::RX_IMU_VN);
    *pdo.mutable_header() = header;
    *pdo.mutable_imuvn_rx_pdo() = imu;

    std::vector<std::uint8_t> payload;
    const advrf::zenoh_plugin::serialization::Ros2CdrSerializer serializer(
        advrf::zenoh_plugin::serialization::Ros2MessageType::Imu);
    assert(serializer.serialize_cycle({pdo}, payload));

    eprosima::fastcdr::FastBuffer buffer(
        reinterpret_cast<char*>(payload.data()), payload.size());
    eprosima::fastcdr::Cdr cdr(buffer);
    cdr.read_encapsulation();

    advrf_interfaces::msg::dds_::Imu_ decoded;
    eprosima::fastcdr::deserialize(cdr, decoded);

    assert(decoded.header().stamp().sec() == 123);
    assert(decoded.header().stamp().nanosec() == 456);
    assert(decoded.header().frame_id() == "imu_1");
    assert(decoded.linear_acceleration().x() == 1.25);
    assert(decoded.angular_velocity().y() == 2.5);
    assert(decoded.orientation().w() == 0.75);
    assert(decoded.imu_ts() == 10);
    assert(decoded.temperature() == 20);
    assert(decoded.digital_in() == 30);
    assert(decoded.fault() == 40);
    assert(decoded.rtt() == 50);

    return 0;
}
