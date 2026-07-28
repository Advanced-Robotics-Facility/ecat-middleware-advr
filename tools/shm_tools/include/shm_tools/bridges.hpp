#pragma once

#include "shm_tools/bridge_inspector.hpp"
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

class ReadBridgePub : public BridgeInspector<SharedPubBridge>
{
public:
    using BridgeInspector::BridgeInspector;

protected:

    void declare() override
    {
        register_queue<iit::advrf::Ec_slave_pdo>("imu", bridge_->imu);
        register_queue<iit::advrf::Ec_slave_pdo>("motor", bridge_->motor);
        register_queue<iit::advrf::Ec_slave_pdo>("gripper", bridge_->gripper);
        register_queue<iit::advrf::Ec_slave_pdo>("force_torque", bridge_->force_torque);
        register_queue<iit::advrf::Ec_slave_pdo>("power_board", bridge_->power_board);
        register_queue<iit::advrf::Ec_slave_pdo>("pump", bridge_->pump);
        register_queue<iit::advrf::Ec_slave_pdo>("valve", bridge_->valve);
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
            bridge_->request);

        register_queue<iit::advrf::Cmd_reply>(
            "reply",
            bridge_->reply);
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
        register_queue<iit::advrf::Motors_PDO_cmd>(
            "motors_pdo",
            bridge_->motors_pdo);
    }
};