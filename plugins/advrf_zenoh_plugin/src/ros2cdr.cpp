#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

#include <string>

#include <advrf_dds_common/converter/converter.hpp>

#include <advrf_interfaces/msg/ForceTorque.hpp>
#include <advrf_interfaces/msg/ForceTorqueCdrAux.hpp>
#include <advrf_interfaces/msg/Gripper.hpp>
#include <advrf_interfaces/msg/GripperCdrAux.hpp>
#include <advrf_interfaces/msg/Imu.hpp>
#include <advrf_interfaces/msg/ImuCdrAux.hpp>
#include <advrf_interfaces/msg/Motor.hpp>
#include <advrf_interfaces/msg/MotorCdrAux.hpp>
#include <advrf_interfaces/msg/PowerBoard.hpp>
#include <advrf_interfaces/msg/PowerBoardCdrAux.hpp>
#include <advrf_interfaces/msg/Pump.hpp>
#include <advrf_interfaces/msg/PumpCdrAux.hpp>
#include <advrf_interfaces/msg/Valve.hpp>
#include <advrf_interfaces/msg/ValveCdrAux.hpp>
#include <sensor_msgs/msg/JointState.hpp>
#include <sensor_msgs/msg/JointStateCdrAux.hpp>

#include "advrf_zenoh_plugin/serialization/cdr_utils.hpp"

namespace advrf::zenoh_plugin::serialization
{
namespace
{

using Pdo = iit::advrf::Ec_slave_pdo;
using Payload = std::vector<std::uint8_t>;

bool has_valid_header(const Pdo& pdo)
{
    return pdo.has_header() && pdo.header().has_stamp();
}

template<typename Message>
void set_header(Message& message, const Pdo& pdo, bool include_frame_id)
{
    const auto& source = pdo.header();
    auto& target = message.header();
    target.stamp().sec() = source.stamp().sec();
    target.stamp().nanosec() = source.stamp().nsec();
    target.frame_id() = include_frame_id ? source.str_id() : std::string{};
}

template<typename Message, typename Append>
bool serialize_aggregate(const std::vector<Pdo>& pdos,
                         Payload& payload,
                         Append&& append)
{
    payload.clear();
    if (pdos.empty()) return false;

    Message message;
    for (const auto& pdo : pdos)
    {
        if (!has_valid_header(pdo) || !append(pdo, message))
            return false;
    }

    set_header(message, pdos.back(), false);
    return ros2cdr::serialize_idl(message, payload);
}

template<typename Message, typename Convert>
bool serialize_device(const std::vector<Pdo>& pdos,
                      Pdo::Type expected_type,
                      Payload& payload,
                      Convert&& convert)
{
    payload.clear();
    if (pdos.size() != 1 || pdos.front().type() != expected_type ||
        !has_valid_header(pdos.front()))
        return false;

    const auto& pdo = pdos.front();
    Message message;
    convert(pdo, message);
    set_header(message, pdo, true);
    return ros2cdr::serialize_idl(message, payload);
}

bool serialize_joint_state(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = sensor_msgs::msg::dds_::JointState_;
    return serialize_aggregate<Message>(pdos, payload,
        [](const Pdo& pdo, Message& message)
        {
            switch (pdo.type())
            {
                case Pdo::RX_CIA402:
                    convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message);
                    break;
                case Pdo::RX_XT_MOTOR:
                    convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message);
                    break;
                case Pdo::RX_MOTOR:
                    convert::dds::from_protobuf(pdo.motor_rx_pdo(), message);
                    break;
                case Pdo::RX_GRIPPER:
                    convert::dds::from_protobuf(pdo.gripper_rx_pdo(), message);
                    break;
                case Pdo::RX_HYQ_KNEE:
                    convert::dds::from_protobuf(pdo.hyqknee_rx_pdo(), message);
                    break;
                default:
                    return false;
            }

            message.name().push_back(pdo.header().str_id());
            return true;
        });
}

bool serialize_motor(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::Motor_;
    return serialize_aggregate<Message>(pdos, payload,
        [](const Pdo& pdo, Message& message)
        {
            switch (pdo.type())
            {
                case Pdo::RX_CIA402:
                    convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message);
                    break;
                case Pdo::RX_XT_MOTOR:
                    convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message);
                    break;
                case Pdo::RX_MOTOR:
                    convert::dds::from_protobuf(pdo.motor_rx_pdo(), message);
                    break;
                default:
                    return false;
            }

            message.name().push_back(pdo.header().str_id());
            return true;
        });
}

bool serialize_valve(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::Valve_;
    return serialize_aggregate<Message>(pdos, payload,
        [](const Pdo& pdo, Message& message)
        {
            if (pdo.type() != Pdo::RX_HYQ_KNEE) return false;
            convert::dds::from_protobuf(pdo.hyqknee_rx_pdo(), message);
            message.name().push_back(pdo.header().str_id());
            return true;
        });
}

bool serialize_gripper(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::Gripper_;
    return serialize_aggregate<Message>(pdos, payload,
        [](const Pdo& pdo, Message& message)
        {
            if (pdo.type() != Pdo::RX_GRIPPER) return false;
            convert::dds::from_protobuf(pdo.gripper_rx_pdo(), message);
            message.name().push_back(pdo.header().str_id());
            return true;
        });
}

bool serialize_imu(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::Imu_;
    return serialize_device<Message>(pdos, Pdo::RX_IMU_VN, payload,
        [](const Pdo& pdo, Message& message)
        {
            convert::dds::from_protobuf(pdo.imuvn_rx_pdo(), message);
        });
}

bool serialize_power_board(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::PowerBoard_;
    return serialize_device<Message>(pdos, Pdo::RX_POW_F28M36, payload,
        [](const Pdo& pdo, Message& message)
        {
            convert::dds::from_protobuf(pdo.powf28m36_rx_pdo(), message);
        });
}

bool serialize_pump(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::Pump_;
    return serialize_device<Message>(pdos, Pdo::RX_HYQ_HPU, payload,
        [](const Pdo& pdo, Message& message)
        {
            convert::dds::from_protobuf(pdo.hyqhpu_rx_pdo(), message);
            message.name() = pdo.header().str_id();
        });
}

bool serialize_force_torque(const std::vector<Pdo>& pdos, Payload& payload)
{
    using Message = advrf_interfaces::msg::dds_::ForceTorque_;
    return serialize_device<Message>(pdos, Pdo::RX_FT6, payload,
        [](const Pdo& pdo, Message& message)
        {
            convert::dds::from_protobuf(pdo.ft6_rx_pdo(), message);
        });
}

} 

bool Ros2CdrSerializer::serialize_cycle(
    const std::vector<Pdo>& pdos,
    Payload& payload) const
{
    switch (message_type_)
    {
        case Ros2MessageType::JointState:
            return serialize_joint_state(pdos, payload);
        case Ros2MessageType::Motor:
            return serialize_motor(pdos, payload);
        case Ros2MessageType::Valve:
            return serialize_valve(pdos, payload);
        case Ros2MessageType::Gripper:
            return serialize_gripper(pdos, payload);
        case Ros2MessageType::Imu:
            return serialize_imu(pdos, payload);
        case Ros2MessageType::PowerBoard:
            return serialize_power_board(pdos, payload);
        case Ros2MessageType::Pump:
            return serialize_pump(pdos, payload);
        case Ros2MessageType::ForceTorque:
            return serialize_force_torque(pdos, payload);
    }

    payload.clear();
    return false;
}

}
