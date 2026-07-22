#pragma once

#include <dds/dds.hpp>
#include "advrf_cyclonedds_plugin/publisher/dds_adapter_publisher.hpp"

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>


using AdapterPublishers=middleware_adapter::message::AdapterPublishers;
using ICallback=middleware_adapter::message::AdapterPublishers::ICallback;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    template<typename Pub>
    void register_publisher(const std::string& robot_name)
    {
        auto pub = std::make_unique<Pub>();
        if (!pub->init(robot_name, dp_))
            throw std::runtime_error("Failed to initialize publisher");

        subscribe(pub.get(), Pub::channels);
        pdo_publishers_.push_back(std::move(pub));
    }

    dds::domain::DomainParticipant& participant() { return dp_; }
    bool init(const RobotConfig& cfg) override {
        dp_ = dds::domain::DomainParticipant(cfg.domain_id);
        register_publisher<ImuPublisher>(cfg.robot_name);
        register_publisher<JointStatePublisher>(cfg.robot_name);
        register_publisher<MotorsPublisher>(cfg.robot_name);
        register_publisher<PowerBoardPublisher>(cfg.robot_name);
        register_publisher<PumpPublisher>(cfg.robot_name);

        return true;
    }

private:
    dds::domain::DomainParticipant dp_{dds::core::null};
    std::vector<std::unique_ptr<ICallback>> pdo_publishers_;
    
};


// pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR
