#include <cassert>
#include <iostream>
#include <cmath>

#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>

void assert_near(double actual, double expected)
{
    constexpr double tolerance = 1e-6;
    assert(std::abs(actual - expected) < tolerance);
}

int main() {
    
    advrf_interfaces::msg::dds_::MotorTxPdo_ input;

    input.pos_ref() = 1.0f;
    input.gain_0() = 0;
    input.gain_1() = 1;
    input.fault_ack() = 6;
    input.ts() = 123;

    iit::advrf::Motor_tx_pdo motor;
    advrf::dds_common::convert::protobuf::from_dds(input, motor);

    assert_near(motor.pos_ref(), 1.0);
    assert(motor.gainp() == input.gain_0());
    assert(motor.gaind() == input.gain_1());
    assert(motor.fault_ack() == input.fault_ack());
    assert(motor.ts() == input.ts());

    std::cout << "Motor Tx convertion (proto-dds) test passed! \n";

    return 0;
}
