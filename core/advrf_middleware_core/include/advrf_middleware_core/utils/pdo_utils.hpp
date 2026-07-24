#pragma once

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
    Pump, 
    Gripper
};

inline void warn_type_mismatch(iit::advrf::Ec_slave_pdo::Type got, PdoExpected expected)
{
    const char* expected_str = 
        expected == PdoExpected::Motor       ? "motor":
        expected == PdoExpected::Imu         ? "IMU":
        expected == PdoExpected::PowerBoard  ? "power board":
        expected == PdoExpected::ForceTorque ? "force torque":
        expected == PdoExpected::Valve       ? "valve":
        expected == PdoExpected::Pump        ? "pump":
                                               "gripper";
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

inline bool is_gripper_type(iit::advrf::Ec_slave_pdo::Type t) 
{
    return t == iit::advrf::Ec_slave_pdo::RX_GRIPPER;
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
        case PdoExpected::Gripper:     return is_gripper_type(t);
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

} 
