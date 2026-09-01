#pragma once

#include "shm_tools/bridge_inspector.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <shm_types.hpp>

namespace advrf::middleware::tools::shm {

class ReadBridgeRx : public BridgeInspector<SharedProtoPubBridge> {
public:
  using Base = BridgeInspector<SharedProtoPubBridge>;
  using Base::Base;

protected:
  void declare() override {
    register_queue<iit::advrf::Ec_slave_pdo>(
        "imu", bridge_->payload.queue_for(DeviceTypeRx::IMU));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "motor", bridge_->payload.queue_for(DeviceTypeRx::MOTOR));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "gripper", bridge_->payload.queue_for(DeviceTypeRx::GRIPPER));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "force_torque", bridge_->payload.queue_for(DeviceTypeRx::FORCE_TORQUE));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "power_board", bridge_->payload.queue_for(DeviceTypeRx::POWER_BOARD));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "pump", bridge_->payload.queue_for(DeviceTypeRx::PUMP));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "valve", bridge_->payload.queue_for(DeviceTypeRx::VALVE));
  }
};

class ReadBridgeTx : public BridgeInspector<SharedProtoSubBridge> {
public:
  using Base = BridgeInspector<SharedProtoSubBridge>;
  using Base::Base;

protected:
  void declare() override {
    register_queue<iit::advrf::Ec_slave_pdo>(
        "motor", bridge_->payload.queue_for(DeviceTypeTx::MOTOR));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "pump", bridge_->payload.queue_for(DeviceTypeTx::PUMP));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "valve", bridge_->payload.queue_for(DeviceTypeTx::VALVE));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "gripper", bridge_->payload.queue_for(DeviceTypeTx::GRIPPER));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "power_board", bridge_->payload.queue_for(DeviceTypeTx::POWERBOARD));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "force_torque", bridge_->payload.queue_for(DeviceTypeTx::FORCETORQUE));
  }
};

class ReadBridgeService : public BridgeInspector<SharedReplBridge> {
public:
  using Base = BridgeInspector<SharedReplBridge>;
  using Base::Base;

protected:
  void declare() override {
    register_queue<iit::advrf::Repl_cmd>("request", bridge_->payload.request);
    register_queue<iit::advrf::Cmd_reply>("reply", bridge_->payload.reply);
  }
};

}