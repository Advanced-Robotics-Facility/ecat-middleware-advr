#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

int main()
{
    iit::advrf::Header header;
    header.set_str_id("motor_7");
    header.mutable_stamp()->set_sec(123);
    header.mutable_stamp()->set_nsec(456);

    iit::advrf::Cia402_rx_pdo motor;
    motor.set_statusword(0x1234);
    motor.set_modes_of_op(8);
    motor.set_motor_pos(1.0F);
    motor.set_motor_vel(2.0F);
    motor.set_link_pos(3.0F);
    motor.set_link_vel(4.0F);
    motor.set_current(5.0F);
    motor.set_torque(6.0F);
    motor.set_demanded_pos(7.0F);
    motor.set_demanded_vel(8.0F);
    motor.set_demanded_torque(9.0F);
    motor.set_demanded_current(10.0F);
    motor.set_control_effort(11.0F);
    motor.set_motor_temp(12.0F);
    motor.set_drive_temp(13);
    motor.set_error_code(14);
    motor.set_error_report("ok");

    iit::advrf::Ec_slave_pdo first;
    first.set_type(iit::advrf::Ec_slave_pdo::RX_CIA402);
    *first.mutable_header() = header;
    *first.mutable_cia402_rx_pdo() = motor;

    iit::advrf::Ec_slave_pdo second = first;
    second.mutable_header()->set_str_id("motor_8");
    second.mutable_header()->mutable_stamp()->set_sec(124);
    second.mutable_header()->mutable_stamp()->set_nsec(457);
    second.mutable_cia402_rx_pdo()->set_motor_pos(101.0F);

    const std::vector<iit::advrf::Ec_slave_pdo> motors{first, second};
    std::vector<std::uint8_t> payload;
    const advrf::zenoh_plugin::serialization::Ros2CdrSerializer serializer(
        advrf::zenoh_plugin::serialization::Ros2MessageType::Motor);
    assert(serializer.serialize_cycle(motors, payload));
    assert(!payload.empty());

    eprosima::fastcdr::FastBuffer buffer(
        reinterpret_cast<char*>(payload.data()), payload.size());
    eprosima::fastcdr::Cdr cdr(buffer);
    cdr.read_encapsulation();

    std::int32_t sec{};
    std::uint32_t nanosec{};
    std::string frame_id;
    std::vector<std::string> name;
    std::vector<std::uint32_t> statusword;
    std::vector<std::int32_t> modes_of_op;
    std::vector<float> motor_pos;
    std::vector<float> motor_vel;
    std::vector<float> link_pos;
    std::vector<float> link_vel;
    std::vector<float> current;
    std::vector<float> torque;
    std::vector<float> demanded_pos;
    std::vector<float> demanded_vel;
    std::vector<float> demanded_torque;
    std::vector<float> demanded_current;
    std::vector<float> control_effort;
    std::vector<float> motor_temp;
    std::vector<std::int32_t> drive_temp;
    std::vector<std::uint32_t> error_code;
    std::vector<std::string> error_report;
    std::vector<std::uint32_t> fault;
    std::vector<std::uint32_t> rtt;

    cdr >> sec >> nanosec >> frame_id;
    cdr >> name >> statusword >> modes_of_op;
    cdr >> motor_pos >> motor_vel >> link_pos >> link_vel;
    cdr >> current >> torque >> demanded_pos >> demanded_vel;
    cdr >> demanded_torque >> demanded_current >> control_effort >> motor_temp;
    cdr >> drive_temp >> error_code >> error_report >> fault >> rtt;

    assert(sec == 124);
    assert(nanosec == 457);
    assert(frame_id.empty());
    assert(name == std::vector<std::string>({"motor_7", "motor_8"}));
    assert(statusword == std::vector<std::uint32_t>({0x1234, 0x1234}));
    assert(modes_of_op == std::vector<std::int32_t>({8, 8}));
    assert(motor_pos == std::vector<float>({1.0F, 101.0F}));
    assert(motor_vel == std::vector<float>({2.0F, 2.0F}));
    assert(link_pos == std::vector<float>({3.0F, 3.0F}));
    assert(link_vel == std::vector<float>({4.0F, 4.0F}));
    assert(current == std::vector<float>({5.0F, 5.0F}));
    assert(torque == std::vector<float>({6.0F, 6.0F}));
    assert(demanded_pos == std::vector<float>({7.0F, 7.0F}));
    assert(demanded_vel == std::vector<float>({8.0F, 8.0F}));
    assert(demanded_torque == std::vector<float>({9.0F, 9.0F}));
    assert(demanded_current == std::vector<float>({10.0F, 10.0F}));
    assert(control_effort == std::vector<float>({11.0F, 11.0F}));
    assert(motor_temp == std::vector<float>({12.0F, 12.0F}));
    assert(drive_temp == std::vector<std::int32_t>({13, 13}));
    assert(error_code == std::vector<std::uint32_t>({14, 14}));
    assert(error_report == std::vector<std::string>({"ok", "ok"}));
    assert(fault == std::vector<std::uint32_t>({0, 0}));
    assert(rtt == std::vector<std::uint32_t>({0, 0}));

    return 0;
}
