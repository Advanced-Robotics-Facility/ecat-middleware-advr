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
#include <advrf_interfaces/msg/ForceTorqueTxPdo.hpp>
#include <advrf_interfaces/msg/ForceTorqueTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/GripperTxPdo.hpp>
#include <advrf_interfaces/msg/GripperTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/PowerBoardTxPdo.hpp>
#include <advrf_interfaces/msg/PowerBoardTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/PumpTxPdo.hpp>
#include <advrf_interfaces/msg/PumpTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/ValveTxPdo.hpp>
#include <advrf_interfaces/msg/ValveTxPdoCdrAux.hpp>
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

}
}

namespace advrf::zenoh_plugin::deserialization
{
namespace
{

using Pdo = iit::advrf::Ec_slave_pdo;
using Payload = std::vector<std::uint8_t>;

template<typename Message, typename Convert>
bool deserialize_vector(const Payload& payload,
                        std::vector<Pdo>& pdos,
                        Convert&& convert)
{
    pdos.clear();
    Message message;
    if (!ros2cdr::deserialize_idl(payload, message) || message.data().empty())
        return false;

    pdos.reserve(message.data().size());
    for (const auto& command : message.data())
    {
        Pdo pdo;
        convert(command, pdo);
        pdos.emplace_back(std::move(pdo));
    }
    return true;
}

bool deserialize_joint(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::MotorTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_CIA402);
            pdo.mutable_header()->set_index(command.ecat_id());
            auto* target = pdo.mutable_cia402_tx_pdo();
            target->set_target_torque(command.tor_ref());
            target->set_target_pos(command.pos_ref());
            target->set_target_vel(command.vel_ref());
            target->set_target_current(command.cur_ref());
            target->set_gain_0(command.gain_0());
            target->set_gain_1(command.gain_1());
            target->set_gain_2(command.gain_2());
            target->set_gain_3(command.gain_3());
            target->set_gain_4(command.gain_4());
        });
}

bool deserialize_motor(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::MotorTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_MOTOR);
            pdo.mutable_header()->set_index(command.ecat_id());
            auto* target = pdo.mutable_motor_tx_pdo();
            target->set_pos_ref(command.pos_ref());
            target->set_fault_ack(command.fault_ack());
            target->set_gainp(static_cast<std::int32_t>(command.gain_0()));
            target->set_gaind(static_cast<std::int32_t>(command.gain_1()));
            target->set_ts(command.ts());
        });
}

bool deserialize_motor_xt(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::MotorTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_XT_MOTOR);
            pdo.mutable_header()->set_index(command.ecat_id());
            auto* target = pdo.mutable_motor_xt_tx_pdo();
            target->set_pos_ref(command.pos_ref());
            target->set_vel_ref(command.vel_ref());
            target->set_tor_ref(command.tor_ref());
            target->set_gain_0(command.gain_0());
            target->set_gain_1(command.gain_1());
            target->set_gain_2(command.gain_2());
            target->set_gain_3(command.gain_3());
            target->set_gain_4(command.gain_4());
            target->set_fault_ack(command.fault_ack());
            target->set_ts(command.ts());
            target->set_op_idx_aux(command.op_idx_aux());
            target->set_aux(command.aux());
        });
}

bool deserialize_valve(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::ValveTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_HYQ_KNEE);
            auto* target = pdo.mutable_hyqknee_tx_pdo();
            target->set_current_ref(command.current_ref());
            target->set_position_ref(command.position_ref());
            target->set_force_ref(command.force_ref());
            target->set_gain_0(command.gain_0());
            target->set_gain_1(command.gain_1());
            target->set_gain_2(command.gain_2());
            target->set_gain_3(command.gain_3());
            target->set_gain_4(command.gain_4());
            target->set_fault_ack(command.fault_ack());
            target->set_ts(command.ts());
            target->set_op_idx_aux(command.op_idx_aux());
            target->set_aux(command.aux());
        });
}

bool deserialize_gripper(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::GripperTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_GRIPPER);
            auto* target = pdo.mutable_gripper_tx_pdo();
            target->set_target_pos(command.target_pos());
            target->set_target_vel(command.target_vel());
            target->set_target_torque(command.target_torque());
            target->set_gain_0(command.gain_0());
            target->set_gain_1(command.gain_1());
            target->set_gain_2(command.gain_2());
            target->set_gain_3(command.gain_3());
            target->set_gain_4(command.gain_4());
        });
}

bool deserialize_pump(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::PumpTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_HYQ_HPU);
            auto* target = pdo.mutable_hyqhpu_tx_pdo();
            target->set_pump_target(command.pump_target());
            target->set_pressure_p_gain(command.pressure_p_gain());
            target->set_pressure_i_gain(command.pressure_i_gain());
            target->set_pressure_d_gain(command.pressure_d_gain());
            target->set_pressure_i_limit(command.pressure_i_limit());
            target->set_fault_ack(command.fault_ack());
            target->set_solenoidout(command.solenoid_out());
            target->set_ts(command.ts());
            target->set_op_idx_aux(command.op_idx_aux());
            target->set_aux(command.aux());
        });
}

bool deserialize_power_board(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::PowerBoardTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_POW_F28M36);
            auto* target = pdo.mutable_powf28m36_tx_pdo();
            target->set_master_command(command.master_command());
            target->set_fault_ack(command.fault_ack());
            target->set_op_idx_aux(command.op_idx_aux());
            target->set_aux(command.aux());
        });
}

bool deserialize_force_torque(const Payload& payload, std::vector<Pdo>& pdos)
{
    using Message = advrf_interfaces::msg::dds_::ForceTorqueTxPdoVector_;
    return deserialize_vector<Message>(payload, pdos,
        [](const auto& command, Pdo& pdo)
        {
            pdo.set_type(Pdo::TX_FT6);
            auto* target = pdo.mutable_ft6_tx_pdo();
            target->set_ts(command.ts());
            target->set_op_idx_aux(command.op_idx_aux());
            target->set_aux(command.aux());
        });
}

}

bool Ros2CdrDeserializer::deserialize_cycle(
    const Payload& payload,
    std::vector<Pdo>& pdos) const
{
    switch (message_type_)
    {
        case Ros2CommandType::Joint:
            return deserialize_joint(payload, pdos);
        case Ros2CommandType::Motor:
            return deserialize_motor(payload, pdos);
        case Ros2CommandType::MotorXt:
            return deserialize_motor_xt(payload, pdos);
        case Ros2CommandType::Valve:
            return deserialize_valve(payload, pdos);
        case Ros2CommandType::Gripper:
            return deserialize_gripper(payload, pdos);
        case Ros2CommandType::Pump:
            return deserialize_pump(payload, pdos);
        case Ros2CommandType::PowerBoard:
            return deserialize_power_board(payload, pdos);
        case Ros2CommandType::ForceTorque:
            return deserialize_force_torque(payload, pdos);
    }

    pdos.clear();
    return false;
}

}

namespace advrf::zenoh_plugin::serialization
{
namespace
{

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
