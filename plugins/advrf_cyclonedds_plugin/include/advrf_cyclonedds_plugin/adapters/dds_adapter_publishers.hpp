#pragma once

#include <dds/dds.hpp>
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publisher.hpp"

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

using AdapterPublishers=middleware_adapter::message::AdapterPublishers;
using ICallback=middleware_adapter::message::AdapterPublishers::ICallback;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    template<typename Pub>
    void register_publisher(const std::string& robot_name,
                            const std::vector<Channel>& channels,
                            const std::vector<uint32_t>& ids_allowed = {})
    {
        auto pub = std::make_unique<Pub>();
        if (!pub->init(robot_name, dp_))
            throw std::runtime_error("Failed to initialize publisher");

        subscribe(pub.get(), channels, ids_allowed);
        pdo_publishers_.push_back(std::move(pub));
    }

    dds::domain::DomainParticipant& participant() { return dp_; }

    // TODO [Hugo]: fit ids allowed to cfg
    bool init(const RobotConfig& cfg) override {
        dp_ = dds::domain::DomainParticipant(cfg.domain_id);
        register_publisher<ImuPublisher>(
            cfg.robot_name,
            {Channel::Imu});

        register_publisher<JointStatePublisher>(
            cfg.robot_name,
            {Channel::Motor, Channel::Gripper},
            {2, 3, 4, 
                          5, 6, 7, 8, 
                          9, 10, 11, 12, 
                          13, 14, 15});

        register_publisher<MotorsPublisher>(
            cfg.robot_name,
            {Channel::Motor});

        register_publisher<PowerBoardPublisher>(
            cfg.robot_name,
            {Channel::PowerBoard});

        register_publisher<PumpPublisher>(
            cfg.robot_name,
            {Channel::Pump});

        return true;
    }

private:
    dds::domain::DomainParticipant dp_{dds::core::null};
    std::vector<std::unique_ptr<ICallback>> pdo_publishers_;
    
};


// pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR
