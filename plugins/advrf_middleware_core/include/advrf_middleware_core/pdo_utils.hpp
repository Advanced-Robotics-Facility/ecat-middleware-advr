#pragma once

#include "msg.hpp"
#include "clock_utils.hpp"
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include <cstring>
#include <iostream>
#include <ctime>
#include <set>

/**
 * Frame layout:
 *   bytes [0..3]  -- uint32_t little-endian (LSB to MSB) payload length
 *   bytes [4..N]  -- serialised iit::advrf::Ec_slave_pdo
 */
namespace pdo_utils {

inline bool parse_frame(const uint8_t* buf, ssize_t n,
                        iit::advrf::Ec_slave_pdo& pdo_out)
{
    // First 4 bytes must be the payload length
    if (n < 4)
        return false;

    // Read the length prefix and check it matches the actual payload size
    uint32_t pb_len = 0;
    std::memcpy(&pb_len, buf, 4);
    if (static_cast<ssize_t>(pb_len + 4) != n)
        return false;

    return pdo_out.ParseFromArray(buf + 4, static_cast<int>(pb_len));
}

enum class PdoExpected { 
    Motor, 
    Imu, 
    PowerBoard, 
    ForceTorque, 
    Valve, 
    Pump 
};

inline void warn_type_mismatch(iit::advrf::Ec_slave_pdo::Type got, PdoExpected expected)
{
    const char* expected_str = 
        expected == PdoExpected::Motor       ? "motor":
        expected == PdoExpected::Imu         ? "IMU":
        expected == PdoExpected::PowerBoard  ? "power board":
        expected == PdoExpected::ForceTorque ? "force torque":
        expected == PdoExpected::Valve       ? "valve":
                                               "pump";
    static std::set<std::pair<iit::advrf::Ec_slave_pdo::Type, PdoExpected>> warned;
    if (!warned.insert({got, expected}).second) return;

    std::cerr << "[pdo_utils] WARNING: received PDO type=" << got
              << " but expected " << expected_str << " data.\n"
              << "            Check your ecat_id mapping in the config.\n";
}

inline bool is_motor_type(iit::advrf::Ec_slave_pdo::Type t)
{
    return t == iit::advrf::Ec_slave_pdo::RX_CIA402   ||
           t == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR ||
           t == iit::advrf::Ec_slave_pdo::RX_MOTOR;
}

inline bool is_imu_type(iit::advrf::Ec_slave_pdo::Type t)
{
    return t == iit::advrf::Ec_slave_pdo::RX_IMU_VN;
}

inline bool is_power_type(iit::advrf::Ec_slave_pdo::Type t)
{
    return t == iit::advrf::Ec_slave_pdo::RX_POW_F28M36;
}

inline bool is_ft_type(iit::advrf::Ec_slave_pdo::Type t)
{
    return t == iit::advrf::Ec_slave_pdo::RX_FT6;
}

inline bool is_valve_type(iit::advrf::Ec_slave_pdo::Type t) 
{
    return t == iit::advrf::Ec_slave_pdo::RX_HYQ_KNEE;
}

inline bool is_pump_type(iit::advrf::Ec_slave_pdo::Type t) 
{
    return t == iit::advrf::Ec_slave_pdo::RX_HYQ_HPU;
}

inline bool is_hub_type(iit::advrf::Ec_slave_pdo::Type t)
{
    return t == iit::advrf::Ec_slave_pdo::DUMMY ||
           t == iit::advrf::Ec_slave_pdo::CLIENT_PIPE;
}

inline bool is_expected_type(iit::advrf::Ec_slave_pdo::Type t, PdoExpected expected)
{
    switch (expected)
    {
        case PdoExpected::Motor:       return is_motor_type(t);
        case PdoExpected::Imu:         return is_imu_type(t);
        case PdoExpected::PowerBoard:  return is_power_type(t);
        case PdoExpected::ForceTorque: return is_ft_type(t);
        case PdoExpected::Valve:       return is_valve_type(t);
        case PdoExpected::Pump:        return is_pump_type(t);
    }
    return false;
}

inline bool check_expected_type(const iit::advrf::Ec_slave_pdo& pdo, PdoExpected expected)
{
    const auto t = pdo.type();
    if (is_hub_type(t)) return false;
    if (is_expected_type(t, expected)) return true;
    warn_type_mismatch(t, expected);
    return false;
}

inline uint64_t extract_timestamp_ns(const iit::advrf::Ec_slave_pdo& pdo)
{
    if (pdo.has_header() && pdo.header().has_stamp())
    {
        const auto& s = pdo.header().stamp();
        const uint64_t mono_ns = static_cast<uint64_t>(s.sec())  * 1'000'000'000ULL + static_cast<uint64_t>(s.nsec());
        return clock_utils::monotonic_to_realtime(mono_ns);
    }
    return clock_utils::monotonic_to_realtime(clock_utils::monotonic_now_ns());
}

// Parses a PDO frame that carries joint state data (CIA402, XT_MOTOR, ...)
inline bool parse_joint_frame(const uint8_t* buf, ssize_t n,
                               double& pos, double& vel, double& eff,
                               uint64_t& timestamp_ns,
                               motor::rt_motor& motor)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::Motor)) 
        return false;

    timestamp_ns = extract_timestamp_ns(pdo);

    if (pdo.type() == iit::advrf::Ec_slave_pdo::RX_CIA402 && pdo.has_cia402_rx_pdo())
    {
        // name?
        const auto& rx = pdo.cia402_rx_pdo();
        pos = rx.link_pos();
        vel = rx.link_vel();
        eff = rx.torque();

        // Required
        motor.statusword       = rx.statusword();
        motor.modes_of_op      = rx.modes_of_op();
        motor.motor_pos        = rx.motor_pos();
        motor.motor_vel        = rx.motor_vel();
        motor.link_pos         = rx.link_pos();
        motor.link_vel         = rx.link_vel();
        motor.current          = rx.current();
        motor.torque           = rx.torque();

        motor.demanded_pos     = rx.demanded_pos();
        motor.demanded_vel     = rx.demanded_vel();
        motor.demanded_torque  = rx.demanded_torque();
        motor.demanded_current = rx.demanded_current();
        motor.control_effort   = rx.control_effort();
        motor.motor_temp       = rx.motor_temp();
        motor.drive_temp       = rx.drive_temp();
        motor.error_code       = rx.error_code();
        motor.error_report     = rx.error_report();
        return true;
    }

    if (pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR && pdo.has_motor_xt_rx_pdo())
    {
        const auto& rx = pdo.motor_xt_rx_pdo();
        pos = rx.link_pos();
        vel = rx.link_vel();
        eff = rx.torque();

        // Required
        motor.motor_pos   = rx.motor_pos();
        motor.motor_vel   = rx.motor_vel();
        motor.link_pos    = rx.link_pos();
        motor.link_vel    = rx.link_vel();
        motor.torque      = rx.torque();
        motor.motor_temp  = rx.motor_temp();
        motor.fault       = rx.fault();
        motor.rtt         = rx.rtt();
        
        return true;
    }

    if (pdo.type() == iit::advrf::Ec_slave_pdo::RX_MOTOR && pdo.has_motor_rx_pdo())
    {
        const auto& rx = pdo.motor_rx_pdo();
        pos = rx.link_pos();
        vel = 0.0;
        eff = static_cast<float>(rx.torque());

        // Required
        motor.link_pos    = rx.link_pos();
        motor.motor_pos   = rx.motor_pos();
        motor.torque      = static_cast<float>(rx.torque());       // Motor_rx_pdo has torque as int32
        motor.motor_temp  = static_cast<float>(rx.temperature());  // Motor_rx_pdo has temp as uint32
        motor.fault       = rx.fault();
        motor.rtt         = rx.rtt();
        return true;
    }

    return false;
}

// Parses a PDO frame that carries IMU data (RX_IMU_VN)
inline bool parse_imu_frame(const uint8_t* buf, ssize_t n, imu::rt_imu_msg& msg)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::Imu)) 
        return false;

    if (pdo.type() != iit::advrf::Ec_slave_pdo::RX_IMU_VN || !pdo.has_imuvn_rx_pdo())
        return false;

    const auto& rx = pdo.imuvn_rx_pdo();
    msg.header.timestamp_ns = extract_timestamp_ns(pdo);

    // Required fields
    msg.angular_velocity    = {rx.x_rate(), rx.y_rate(), rx.z_rate()};
    msg.linear_acceleration = {rx.x_acc(),  rx.y_acc(),  rx.z_acc()};

    msg.orientation         = {rx.x_quat(), rx.y_quat(), rx.z_quat(), rx.w_quat()};
    msg.imu_ts              = rx.imu_ts();
    msg.temperature         = rx.temperature();
    msg.digital_in          = rx.digital_in();
    msg.fault               = rx.fault();
    msg.rtt                 = rx.rtt();
    
    return true;
}

// Parses a PDO frame that carries PowerBoard data 
inline bool parse_power_frame(const uint8_t* buf, ssize_t n, power_board::rt_power_board_msg& msg)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::PowerBoard)) 
        return false;

    msg.header.timestamp_ns = extract_timestamp_ns(pdo);

    if (pdo.type() == iit::advrf::Ec_slave_pdo::RX_POW_F28M36 && pdo.has_powf28m36_rx_pdo())
    {
        const auto& rx = pdo.powf28m36_rx_pdo();

        msg.v_batt         = rx.v_batt();
        msg.v_load         = rx.v_load();
        msg.i_load         = rx.i_load();
        msg.temperature    = rx.temp_pcb();
        msg.temp_heatsink  = rx.temp_heatsink();
        msg.temp_batt      = rx.temp_batt();
        msg.status         = rx.status();
        msg.fault          = rx.fault();
        msg.rtt            = static_cast<uint32_t>(rx.rtt());
        msg.op_idx_ack     = rx.op_idx_ack();
        msg.aux            = rx.aux();
        
        return true;
    }

    return false;
}

// Parses a PDO frame that carries ForceTorque sensor data 
inline bool parse_ft_frame(const uint8_t* buf, ssize_t n, force_torque::rt_force_torque_msg& msg)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::ForceTorque)) 
        return false;

    if (pdo.type() != iit::advrf::Ec_slave_pdo::RX_FT6 || !pdo.has_ft6_rx_pdo())
        return false;

    const auto& rx = pdo.ft6_rx_pdo();

    msg.header.timestamp_ns = extract_timestamp_ns(pdo);

    // Required
    msg.wrench.force        = {rx.force_x(), rx.force_y(), rx.force_z()};
    msg.wrench.torque       = {rx.torque_x(), rx.torque_y(), rx.torque_z()};

    msg.fault               = rx.fault();
    msg.rtt                 = rx.rtt();
    msg.op_idx_ack          = rx.op_idx_ack();
    msg.aux                 = rx.aux();
    
    return true;
}

// Parses a PDO frame that carries Valve data 
inline bool parse_valve_frame(const uint8_t* buf, ssize_t n, valve::rt_valve& msg)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::Valve)) 
        return false;

    if (pdo.type() != iit::advrf::Ec_slave_pdo::RX_HYQ_KNEE || !pdo.has_hyqknee_rx_pdo())
        return false;

    const auto& rx = pdo.hyqknee_rx_pdo();

    msg.header.timestamp_ns = extract_timestamp_ns(pdo);

    msg.encoder_position = rx.encoder_position();
    msg.force            = rx.force();
    msg.pressure1        = rx.pressure_1();
    msg.pressure2        = rx.pressure_2();
    msg.current          = rx.current();
    msg.temperature      = rx.temperature();
    msg.fault            = rx.fault();
    msg.rtt              = rx.rtt();
    msg.op_idx_ack       = rx.op_idx_ack();
    msg.aux              = rx.aux();
    msg.current_ref_fb   = rx.current_ref_fb();
    msg.position_ref_fb  = rx.position_ref_fb();
    msg.force_ref_fb     = rx.force_ref_fb();

    return true;
}

// Parses a PDO frame that carries Pump data 
inline bool parse_pump_frame(const uint8_t* buf, ssize_t n, pump::rt_pump_msg& msg)
{
    iit::advrf::Ec_slave_pdo pdo;
    if (!parse_frame(buf, n, pdo))
        return false;

    if (!check_expected_type(pdo, PdoExpected::Pump)) 
        return false;

    if (pdo.type() != iit::advrf::Ec_slave_pdo::RX_HYQ_HPU || !pdo.has_hyqhpu_rx_pdo())
        return false;

    const auto& rx = pdo.hyqhpu_rx_pdo();

    msg.header.timestamp_ns = extract_timestamp_ns(pdo);

    msg.motor_current       = rx.motor_current();
    msg.motor_speed         = rx.motor_speed();
    msg.pressure1           = rx.pressure1();
    msg.pressure2           = rx.pressure2();
    msg.temperature         = rx.temperature();
    msg.mosfet_temperature  = rx.mosfet_temperature();
    msg.motor_temperature   = rx.motor_temperature();
    msg.fault               = rx.fault();
    msg.rtt                 = rx.rtt();
    msg.op_idx_ack          = rx.op_idx_ack();
    msg.aux                 = rx.aux();
    
    return true;
}

} 
