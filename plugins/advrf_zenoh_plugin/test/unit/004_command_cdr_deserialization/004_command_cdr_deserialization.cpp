#include <cassert>
#include <cstdint>
#include <vector>

#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/ForceTorqueTxPdo.hpp>
#include <advrf_interfaces/msg/ForceTorqueTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/GripperTxPdo.hpp>
#include <advrf_interfaces/msg/GripperTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/PowerBoardTxPdo.hpp>
#include <advrf_interfaces/msg/PowerBoardTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/PumpTxPdo.hpp>
#include <advrf_interfaces/msg/PumpTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/ValveTxPdo.hpp>
#include <advrf_interfaces/msg/ValveTxPdoCdrAux.hpp>

#include "advrf_zenoh_plugin/serialization/cdr_utils.hpp"
#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

namespace
{
using Pdo = iit::advrf::Ec_slave_pdo;
using advrf::zenoh_plugin::deserialization::Ros2CdrDeserializer;
using advrf::zenoh_plugin::deserialization::Ros2CommandType;

template<typename Message>
std::vector<std::uint8_t> serialize(const Message& message)
{
    std::vector<std::uint8_t> payload;
    assert(advrf::zenoh_plugin::serialization::ros2cdr::serialize_idl(
        message, payload));
    return payload;
}
}

int main()
{
    advrf_interfaces::msg::dds_::MotorTxPdoVector_ joints;
    auto& joint = joints.data().emplace_back();
    joint.ecat_id(42);
    joint.tor_ref(1.0F);
    joint.pos_ref(2.0F);
    joint.vel_ref(3.0F);
    joint.cur_ref(4.0F);
    joint.gain_0(5.0F);

    std::vector<Pdo> pdos;
    const Ros2CdrDeserializer joint_decoder(Ros2CommandType::Joint);
    assert(joint_decoder.deserialize_cycle(serialize(joints), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_CIA402);
    assert(pdos.front().header().index() == 42);
    assert(pdos.front().cia402_tx_pdo().target_pos() == 2.0F);
    assert(pdos.front().cia402_tx_pdo().gain_0() == 5.0F);

    advrf_interfaces::msg::dds_::PumpTxPdo_ pump;
    pump.ecat_id(7);
    pump.pump_target(10.0F);
    pump.pressure_p_gain(11.0F);
    pump.solenoid_out(12);
    pump.op_idx_aux(13);

    const Ros2CdrDeserializer pump_decoder(Ros2CommandType::Pump);
    assert(pump_decoder.deserialize_cycle(serialize(pump), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_HYQ_HPU);
    assert(pdos.front().header().index() == 7);
    assert(pdos.front().hyqhpu_tx_pdo().pump_target() == 10.0F);
    assert(pdos.front().hyqhpu_tx_pdo().solenoidout() == 12);
    assert(pdos.front().hyqhpu_tx_pdo().op_idx_aux() == 13);

    advrf_interfaces::msg::dds_::ValveTxPdoVector_ valves;
    auto& valve = valves.data().emplace_back();
    valve.ecat_id(8);
    valve.position_ref(20.0F);

    const Ros2CdrDeserializer valve_decoder(Ros2CommandType::Valve);
    assert(valve_decoder.deserialize_cycle(serialize(valves), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_HYQ_KNEE);
    assert(pdos.front().header().index() == 8);
    assert(pdos.front().hyqknee_tx_pdo().position_ref() == 20.0F);

    advrf_interfaces::msg::dds_::GripperTxPdoVector_ grippers;
    auto& gripper = grippers.data().emplace_back();
    gripper.ecat_id(9);
    gripper.target_pos(30.0F);

    const Ros2CdrDeserializer gripper_decoder(Ros2CommandType::Gripper);
    assert(gripper_decoder.deserialize_cycle(serialize(grippers), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_GRIPPER);
    assert(pdos.front().header().index() == 9);
    assert(pdos.front().gripper_tx_pdo().target_pos() == 30.0F);

    advrf_interfaces::msg::dds_::PowerBoardTxPdo_ power_board;
    power_board.ecat_id(10);
    power_board.master_command(40);

    const Ros2CdrDeserializer power_board_decoder(Ros2CommandType::PowerBoard);
    assert(power_board_decoder.deserialize_cycle(serialize(power_board), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_POW_F28M36);
    assert(pdos.front().header().index() == 10);
    assert(pdos.front().powf28m36_tx_pdo().master_command() == 40);

    advrf_interfaces::msg::dds_::ForceTorqueTxPdo_ force_torque;
    force_torque.ecat_id(11);
    force_torque.op_idx_aux(50);

    const Ros2CdrDeserializer force_torque_decoder(Ros2CommandType::ForceTorque);
    assert(force_torque_decoder.deserialize_cycle(serialize(force_torque), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_FT6);
    assert(pdos.front().header().index() == 11);
    assert(pdos.front().ft6_tx_pdo().op_idx_aux() == 50);

    assert(!pump_decoder.deserialize_cycle({}, pdos));
    assert(pdos.empty());
    return 0;
}
