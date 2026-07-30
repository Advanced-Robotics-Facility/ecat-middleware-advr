#pragma once

#include "shm_tools/bridge_inspector.hpp"

#include <ecat_master_future/shm/bridge_struct.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

class ReadBridgePub : public BridgeInspector<SharedPubBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

    void declare() override
    {
        register_queue<iit::advrf::Ec_slave_pdo>("imu", bridge_->payload.imu);
        register_queue<iit::advrf::Ec_slave_pdo>("motor", bridge_->payload.motor);
        register_queue<iit::advrf::Ec_slave_pdo>("gripper", bridge_->payload.gripper);
        register_queue<iit::advrf::Ec_slave_pdo>("force_torque", bridge_->payload.force_torque);
        register_queue<iit::advrf::Ec_slave_pdo>("power_board", bridge_->payload.power_board);
        register_queue<iit::advrf::Ec_slave_pdo>("pump", bridge_->payload.pump);
        register_queue<iit::advrf::Ec_slave_pdo>("valve", bridge_->payload.valve);
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

class ReadBridgeSub : public BridgeInspector<SharedSubBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

     void declare() override
    {
        register_queue<iit::advrf::Repl_cmd>(
            "request",
            bridge_->payload.request);
    }
};