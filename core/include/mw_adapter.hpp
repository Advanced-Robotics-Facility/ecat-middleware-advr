#pragma once

#include "msg.hpp"
#include "robot_config.hpp"

class MiddlewareAdapter {
public:
    virtual ~MiddlewareAdapter() = default;

    virtual bool init(const RobotConfig& cfg) = 0;

    virtual void publish(const joint_state::rt_joint_state_msg&)     = 0;
    virtual void publish(const imu::rt_imu_msg&)                     = 0;
    virtual void publish(const force_torque::rt_force_torque_msg&)   = 0;
    virtual void publish(const motor::rt_motor_msg&)                 = 0;
    virtual void publish(const power_board::rt_power_board_msg&)     = 0;
    virtual void publish(const pump::rt_pump_msg&)                   = 0;
    virtual void publish(const valve::rt_valve_msg&)                 = 0;
    virtual void publish(const gripper::rt_gripper_msg&)             = 0;
};