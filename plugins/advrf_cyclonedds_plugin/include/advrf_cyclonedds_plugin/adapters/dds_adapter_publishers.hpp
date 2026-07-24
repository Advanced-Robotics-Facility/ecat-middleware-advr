#pragma once

#include <dds/dds.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_cyclonedds_plugin/config/config_topics.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_bridges.hpp"

using AdapterPublishers=middleware_adapter::message::AdapterPublishers;
using IPublisher=middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription=middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    // TODO [Hugo]: fit ids allowed to cfg
    bool init(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& dp) {
        register_publisher<ImuPublisher>(
                {Channel::Imu}, 
                {1})
            .init(config_topics.state.imu(), dp);

        register_publisher<JointStatePublisher>(
                {Channel::Motor, Channel::Gripper}, 
                {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})
            .init(config_topics.state.jointState(), dp);

        register_publisher<MotorsPublisher>(
                {Channel::Motor},
                {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})
            .init(config_topics.state.motor(), dp);

        register_publisher<PowerBoardPublisher>(
                {Channel::PowerBoard},
                {})
            .init(config_topics.state.powerBoard(), dp);

        register_publisher<PumpPublisher>(
                {Channel::Pump}, 
                {})
            .init(config_topics.state.pump(), dp);
            
        register_publisher<ForceTorquePublisher>(
                {Channel::ForceTorque}, 
                {})
            .init(config_topics.state.forceTorque(), dp);


        
        return true;
    }

    private:
};


// pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR
