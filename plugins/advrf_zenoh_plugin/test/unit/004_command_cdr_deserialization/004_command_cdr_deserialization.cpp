#include <cassert>
#include <cstdint>
#include <vector>

#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdoCdrAux.hpp>
#include <advrf_interfaces/msg/PumpTxPdo.hpp>
#include <advrf_interfaces/msg/PumpTxPdoCdrAux.hpp>

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

    advrf_interfaces::msg::dds_::PumpTxPdoVector_ pumps;
    auto& pump = pumps.data().emplace_back();
    pump.pump_target(10.0F);
    pump.pressure_p_gain(11.0F);
    pump.solenoid_out(12);
    pump.op_idx_aux(13);

    const Ros2CdrDeserializer pump_decoder(Ros2CommandType::Pump);
    assert(pump_decoder.deserialize_cycle(serialize(pumps), pdos));
    assert(pdos.size() == 1);
    assert(pdos.front().type() == Pdo::TX_HYQ_HPU);
    assert(pdos.front().hyqhpu_tx_pdo().pump_target() == 10.0F);
    assert(pdos.front().hyqhpu_tx_pdo().solenoidout() == 12);
    assert(pdos.front().hyqhpu_tx_pdo().op_idx_aux() == 13);

    assert(!pump_decoder.deserialize_cycle({}, pdos));
    assert(pdos.empty());
    return 0;
}
