#pragma once

#include <ecat_master_pb/protobuf/ecat_pdo.pb.h>
#include <protobuf/repl_cmd.pb.h>
#include <shm_types.hpp>

#include "shm_tools/bridge_inspector.hpp"

namespace advrf::middleware::tools::shm {

class ReadBridgeRx : public BridgeInspector<SharedProtoPubBridge> {
public:
  using Base = BridgeInspector<SharedProtoPubBridge>;
  using Base::Base;

protected:
  void declare() override {
    register_queue<iit::advrf::Ec_slave_pdo>(
        "imu", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "motor", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "gripper", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "force_torque", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "power_board", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "pump", bridge_->payload.rx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "valve", bridge_->payload.rx_queue));
  }
};

class ReadBridgeTx : public BridgeInspector<SharedProtoSubBridge> {
public:
  using Base = BridgeInspector<SharedProtoSubBridge>;
  using Base::Base;

protected:
  void declare() override {
    register_queue<iit::advrf::Ec_slave_pdo>(
        "motor", bridge_->payload.tx_queue));

    register_queue<iit::advrf::Ec_slave_pdo>(
        "pump", bridge_->payload.tx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "valve", bridge_->payload.tx_queue);

    register_queue<iit::advrf::Ec_slave_pdo>(
        "gripper", bridge_->payload.tx_queue);

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