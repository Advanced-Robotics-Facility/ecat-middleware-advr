#include <cassert>
#include <iostream>
#include <cmath>

#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_interfaces/msg/Imu.hpp>

void assert_near(double actual, double expected)
{
    constexpr double tolerance = 1e-6;
    assert(std::abs(actual - expected) < tolerance);
}

int main() {
    
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

    advrf_interfaces::msg::dds_::Imu_ output;
    convert::dds::from_protobuf(input, output);

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

    std::cout << "Imu convertion test passed! \n";

    return 0;
}
