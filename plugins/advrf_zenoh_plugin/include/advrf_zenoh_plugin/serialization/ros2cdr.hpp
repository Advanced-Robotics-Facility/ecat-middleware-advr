#pragma once

#include <cstdint>
#include <vector>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

namespace advrf::zenoh_plugin::serialization
{

enum class Ros2MessageType
{
    JointState,
    Motor,
    Valve,
    Gripper,
    Imu,
    PowerBoard,
    Pump,
    ForceTorque,
};

class Ros2CdrSerializer
{
public:
    explicit Ros2CdrSerializer(Ros2MessageType message_type)
        : message_type_(message_type)
    {}

    bool serialize_cycle(
        const std::vector<iit::advrf::Ec_slave_pdo>& pdos,
        std::vector<std::uint8_t>& payload) const;

private:
    Ros2MessageType message_type_;
};

}
