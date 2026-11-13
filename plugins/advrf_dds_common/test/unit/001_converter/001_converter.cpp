#include <cassert>
#include <cmath>
#include <iostream>

#include <advrf_ros2_interfaces/msg/Imu.hpp>
#include <advrf_middleware_core/utils/clock_utils.hpp>

#include "advrf_dds_common/converter/converter.hpp"

void assert_near(double actual, double expected)
{
    constexpr double tolerance = 1e-6;
    assert(std::abs(actual - expected) < tolerance);
}

int main() {
    advrf::middleware::clock::init();
    
    iit::advrf::ImuVN_rx_pdo input;

    input.set_x_rate(1.0F);
    input.set_y_rate(1.1F);
    input.set_z_rate(1.2F);

    input.set_x_acc(2.0F);
    input.set_y_acc(2.1F);
    input.set_z_acc(2.2F);

    input.set_x_quat(0.1F);
    input.set_y_quat(0.2F);
    input.set_z_quat(0.3F);
    input.set_w_quat(0.9F);

    input.set_imu_ts(123);
    input.set_temperature(30);
    input.set_digital_in(4);
    input.set_fault(0);
    input.set_rtt(50);

    advrf_ros2_interfaces::msg::dds_::Imu_ output;
    advrf::dds_common::convert::dds::from_protobuf(input, output);

    assert_near(output.angular_velocity().x(), 1.0);
    assert_near(output.angular_velocity().y(), 1.1);
    assert_near(output.angular_velocity().z(), 1.2);

    assert_near(output.linear_acceleration().x(), 2.0);
    assert_near(output.linear_acceleration().y(), 2.1);
    assert_near(output.linear_acceleration().z(), 2.2);

    assert_near(output.orientation().x(), 0.1);
    assert_near(output.orientation().y(), 0.2);
    assert_near(output.orientation().z(), 0.3);
    assert_near(output.orientation().w(), 0.9);

    assert(output.imu_ts() == 123);
    assert(output.temperature() == 30);
    assert(output.digital_in() == 4);
    assert(output.fault() == 0);
    assert(output.rtt() == 50);

    iit::advrf::Ec_slave_pdo stamped_pdo;
    stamped_pdo.mutable_header()->mutable_stamp()->set_sec(123);
    stamped_pdo.mutable_header()->mutable_stamp()->set_nsec(456);

    std_msgs::msg::dds_::Header_ stamped_header;
    advrf::dds_common::convert::dds::from_protobuf(stamped_pdo, stamped_header);

    const auto expected_stamp_ns =
        advrf::middleware::clock::monotonic_to_realtime(123'000'000'456ULL);
    assert(stamped_header.stamp().sec() ==
           static_cast<int32_t>(expected_stamp_ns / 1'000'000'000ULL));
    assert(stamped_header.stamp().nanosec() ==
           static_cast<uint32_t>(expected_stamp_ns % 1'000'000'000ULL));

    iit::advrf::Ec_slave_pdo zero_stamp_pdo;
    zero_stamp_pdo.mutable_header()->mutable_stamp();
    const auto before_fallback_ns = advrf::middleware::clock::monotonic_to_realtime(
        advrf::middleware::clock::monotonic_now_ns());

    std_msgs::msg::dds_::Header_ fallback_header;
    advrf::dds_common::convert::dds::from_protobuf(zero_stamp_pdo, fallback_header);

    const auto after_fallback_ns = advrf::middleware::clock::monotonic_to_realtime(
        advrf::middleware::clock::monotonic_now_ns());
    const auto fallback_stamp_ns =
        static_cast<uint64_t>(fallback_header.stamp().sec()) * 1'000'000'000ULL +
        fallback_header.stamp().nanosec();
    assert(fallback_stamp_ns >= before_fallback_ns);
    assert(fallback_stamp_ns <= after_fallback_ns);

    std::cout << "Imu convertion test passed! \n";

    return 0;
}
