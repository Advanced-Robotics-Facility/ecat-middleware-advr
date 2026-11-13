#include <stdexcept>

// Keep regression checks enabled in Release builds too.
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)
#include <cmath>
#include <iostream>

#include <advrf_ros2_interfaces/msg/MotorTxPdo.hpp>

#include "advrf_dds_common/converter/converter.hpp"

void assert_near(double actual, double expected)
{
    constexpr double tolerance = 1e-6;
    CHECK(std::abs(actual - expected) < tolerance);
}

int main() {
    
    advrf_ros2_interfaces::msg::dds_::MotorTxPdo_ input;

    input.pos_ref() = 1.0f;
    input.gain_0() = 0;
    input.gain_1() = 1;
    input.fault_ack() = 6;
    input.ts() = 123;

    iit::advrf::Motor_tx_pdo motor;
    advrf::dds_common::convert::protobuf::from_dds(input, motor);

    assert_near(motor.pos_ref(), 1.0);
    CHECK(motor.gainp() == input.gain_0());
    CHECK(motor.gaind() == input.gain_1());
    CHECK(motor.fault_ack() == input.fault_ack());
    CHECK(motor.ts() == input.ts());

    using Pdo = iit::advrf::Ec_slave_pdo;
    input.ecat_id() = 42;
    input.vel_ref() = 2.0f;
    input.tor_ref() = 3.0f;
    input.cur_ref() = 4.0f;
    input.gain_2() = 2.0f;
    input.gain_3() = 3.0f;
    input.gain_4() = 4.0f;
    input.op_idx_aux() = 7;
    input.aux() = 8.0f;
    Pdo envelope;
    const auto check_envelope = [&](Pdo::Type rx, Pdo::Type tx) {
        advrf::dds_common::convert::protobuf::from_dds(input, rx, envelope);
        CHECK(envelope.type() == tx);
        CHECK(envelope.has_header());
        CHECK(envelope.header().has_index());
        CHECK(envelope.header().index() == 42);
        // Exercise the Protobuf wire format passed through shared memory.
        Pdo decoded;
        CHECK(decoded.ParseFromString(envelope.SerializeAsString()));
        CHECK(decoded.type() == tx);
        CHECK(decoded.header().index() == 42);
    };
    check_envelope(Pdo::RX_MOTOR, Pdo::TX_MOTOR);
    CHECK(envelope.has_motor_tx_pdo());
    assert_near(envelope.motor_tx_pdo().pos_ref(), 1.0);
    check_envelope(Pdo::RX_XT_MOTOR, Pdo::TX_XT_MOTOR);
    CHECK(envelope.has_motor_xt_tx_pdo());
    CHECK(!envelope.has_motor_tx_pdo());
    const auto& xt = envelope.motor_xt_tx_pdo();
    assert_near(xt.pos_ref(), 1.0);
    assert_near(xt.vel_ref(), 2.0);
    assert_near(xt.tor_ref(), 3.0);
    assert_near(xt.gain_4(), 4.0);
    CHECK(xt.ts() == 123);
    CHECK(xt.fault_ack() == 6);
    CHECK(xt.op_idx_aux() == 7);
    assert_near(xt.aux(), 8.0);
    check_envelope(Pdo::RX_CIA402, Pdo::TX_CIA402);
    CHECK(envelope.has_cia402_tx_pdo());
    CHECK(!envelope.has_motor_xt_tx_pdo());
    assert_near(envelope.cia402_tx_pdo().target_pos(), 1.0);
    assert_near(envelope.cia402_tx_pdo().target_vel(), 2.0);
    assert_near(envelope.cia402_tx_pdo().target_current(), 4.0);
    check_envelope(Pdo::TX_MOTOR, Pdo::TX_MOTOR);
    check_envelope(Pdo::TX_XT_MOTOR, Pdo::TX_XT_MOTOR);
    check_envelope(Pdo::TX_CIA402, Pdo::TX_CIA402);
    input.ecat_id() = 0;
    advrf::dds_common::convert::protobuf::from_dds(input, Pdo::RX_XT_MOTOR, envelope);
    CHECK(envelope.header().has_index());
    CHECK(envelope.header().index() == 0);
    advrf::dds_common::convert::protobuf::from_dds(input, Pdo::RX_IMU_VN, envelope);
    CHECK(envelope.type() == Pdo::_UNSPECIFIED);
    CHECK(!envelope.has_header());
    CHECK(!envelope.has_motor_xt_tx_pdo());
    input.ecat_id() = -1;
    advrf::dds_common::convert::protobuf::from_dds(input, Pdo::RX_XT_MOTOR, envelope);
    CHECK(envelope.type() == Pdo::_UNSPECIFIED);
    CHECK(!envelope.has_header());

    std::cout << "Motor Tx convertion (proto-dds) test passed! \n";

    return 0;
}
