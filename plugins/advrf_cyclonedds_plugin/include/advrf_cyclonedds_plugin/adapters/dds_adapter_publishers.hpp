#pragma once

#include <dds/dds.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_bridges.hpp"

using AdapterPublishers=middleware_adapter::message::AdapterPublishers;
using ICallback=middleware_adapter::message::AdapterPublishers::ICallback;
using Subscription=middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    dds::domain::DomainParticipant& participant() { return dp_; }

    // TODO [Hugo]: fit ids allowed to cfg
    bool init(const RobotConfig& cfg) override {
        dp_ = dds::domain::DomainParticipant(cfg.domain_id);
        register_callback<ImuPublisher>(
                {Channel::Imu}, 
                {})
            .init(cfg.robot_name, dp_);

        register_callback<JointStatePublisher>(
                {Channel::Motor, Channel::Gripper}, 
                {})
            .init(cfg.robot_name, dp_);

        register_callback<MotorsPublisher>(
                {Channel::Motor},
                {})
            .init(cfg.robot_name, dp_);

        register_callback<PowerBoardPublisher>(
                {Channel::PowerBoard},
                {})
            .init(cfg.robot_name, dp_);

        register_callback<PumpPublisher>(
                {Channel::Pump}, 
                {})
            .init(cfg.robot_name, dp_);

        register_callback<ForceTorquePublisher>(
                {Channel::ForceTorque}, 
                {})
            .init(cfg.robot_name, dp_);

        return true;
    }

private:
    dds::domain::DomainParticipant dp_{dds::core::null};
    std::vector<std::unique_ptr<ICallback>> pdo_publishers_;
    
};


// pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR
