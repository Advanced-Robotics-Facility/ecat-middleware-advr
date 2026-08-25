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
    input.vel_ref() = 2.0f;
    input.tor_ref() = 3.0f;
    input.gain_0() = 0.1f;
    input.gain_1() = 0.2f;
    input.gain_2() = 0.3f;
    input.gain_3() = 0.4f;
    input.gain_4() = 0.5f;
    input.fault_ack() = 6;
    input.ts() = 123;
    input.op_idx_aux() = 7;
    input.aux() = 8.0F;

    iit::advrf::Ec_slave_pdo output;
    convert::protobuf::from_dds(
        input, output, iit::advrf::Ec_slave_pdo::TX_XT_MOTOR);

    assert(output.type() == iit::advrf::Ec_slave_pdo::TX_XT_MOTOR);
    assert(output.has_motor_xt_tx_pdo());

    const auto& motor = output.motor_xt_tx_pdo();

    assert_near(motor.pos_ref(), 1.0f);
    assert_near(motor.vel_ref(), 2.0f);
    assert_near(motor.tor_ref(), 3.0f);
    assert_near(motor.gain_0(), 0.1f);
    assert_near(motor.gain_1(), 0.2f);
    assert_near(motor.gain_2(), 0.3f);
    assert_near(motor.gain_3(), 0.4f);
    assert_near(motor.gain_4(), 0.5f);
    assert(motor.fault_ack() == 6);
    assert(motor.ts() == 123);
    assert(motor.op_idx_aux() == 7);
    assert(motor.aux() == 8.0F);

    std::cout << "Motor XtTx convertion (proto-dds) test passed! \n";

    return 0;
}
