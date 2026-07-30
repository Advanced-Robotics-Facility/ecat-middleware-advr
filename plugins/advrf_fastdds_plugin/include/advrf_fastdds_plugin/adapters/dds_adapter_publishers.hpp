#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_fastdds_plugin/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

using AdapterPublishers = middleware_adapter::message::AdapterPublishers;
using IPublisher = middleware_adapter::message::AdapterPublishers::IPublisher;
using Subscription = middleware_adapter::message::AdapterPublishers::Subscription;

class DDSAdapterPublishers : public AdapterPublishers {
public:
    DDSAdapterPublishers() = default;
    ~DDSAdapterPublishers() override = default;

    bool init(const config::ConfigTopics& config_topics, 
              const RobotConfig& robot_config,
              eprosima::fastdds::dds::DomainParticipant* dp);

    private:
};
