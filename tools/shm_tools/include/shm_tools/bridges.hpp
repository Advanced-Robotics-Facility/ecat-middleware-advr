#pragma once

#include "shm_tools/bridge_inspector.hpp"

#include <shm_types.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

class ReadBridgePub : public BridgeInspector<SharedProtoPubBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

    void declare() override
    {
        register_queue<iit::advrf::Ec_slave_pdo>("imu", bridge_->payload.queue_for(DeviceType::IMU));
        register_queue<iit::advrf::Ec_slave_pdo>("motor", bridge_->payload.queue_for(DeviceType::MOTOR));
        register_queue<iit::advrf::Ec_slave_pdo>("gripper", bridge_->payload.queue_for(DeviceType::GRIPPER));
        register_queue<iit::advrf::Ec_slave_pdo>("force_torque", bridge_->payload.queue_for(DeviceType::FORCE_TORQUE));
        register_queue<iit::advrf::Ec_slave_pdo>("power_board", bridge_->payload.queue_for(DeviceType::POWER_BOARD));
        register_queue<iit::advrf::Ec_slave_pdo>("pump", bridge_->payload.queue_for(DeviceType::PUMP));
        register_queue<iit::advrf::Ec_slave_pdo>("valve", bridge_->payload.queue_for(DeviceType::VALVE));
    }
};


#include "shm_tools/bridge_inspector.hpp"
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class ReadBridgeRepl : public BridgeInspector<SharedReplBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

    void declare() override
    {
        register_queue<iit::advrf::Repl_cmd>(
            "request",
            bridge_->payload.request);

        register_queue<iit::advrf::Cmd_reply>(
            "reply",
            bridge_->payload.reply);
    }
};


#include "shm_tools/bridge_inspector.hpp"
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class ReadBridgeSub : public BridgeInspector<SharedProtoSubBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

     void declare() override
    {
        // TODO: generalize
        register_queue<iit::advrf::Ec_slave_pdo >(
            "motor",
            bridge_->payload.queue_for(DeviceTypeTx::MOTOR));
    }
};