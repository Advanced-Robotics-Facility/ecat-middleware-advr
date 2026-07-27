#pragma once

#include <dds/dds.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_cyclonedds_plugin/config/config_topics.hpp>

using AdapterPublishers=middleware_adapter::message::AdapterPublishers;
using IPublisher=middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription=middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    bool init(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& dp);

    private:
};


// pdo.type() == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR
