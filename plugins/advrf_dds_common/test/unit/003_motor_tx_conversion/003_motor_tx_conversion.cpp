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
    input.gainP() = 0;
    input.gainD() = 1;
    input.fault_ack() = 6;
    input.ts() = 123;

    iit::advrf::Ec_slave_pdo output;
    convert::protobuf::from_dds(input, output);

    assert(output.type() == iit::advrf::Ec_slave_pdo::TX_MOTOR);
    assert(output.has_motor_tx_pdo());

    const auto& motor = output.motor_tx_pdo();

    assert_near(motor.pos_ref(), 1.0);
    assert(motor.gainp() == 0);
    assert(motor.gaind() == 1);
    assert(motor.fault_ack() == 6);
    assert(motor.ts() == 123);

    std::cout << "Motor Tx convertion (proto-dds) test passed! \n";

    return 0;
}
