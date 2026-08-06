#pragma once

#include <string>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/adapters/adapter_publishers.hpp>

#include "advrf_zenoh_plugin/publisher/zenoh_publisher.hpp"

namespace advrf::zenoh_plugin
{

class ZenohAdapterBridgePublisher
    : public middleware_adapter::message::AdapterPublishers::IPublisher
{
public:
    using Pdo = iit::advrf::Ec_slave_pdo;

    bool init(zenoh::Session& session, const std::string& key)
    {
        try {
            publisher_.emplace(session, key);
            LOG_INFO("Topic Created: {}", key);
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to declare Zenoh publisher '{}': {}", key, error.what());
            return false;
        }
    }

    void begin_cycle() override
    {
        messages_.clear();
    }

    void consume(const Pdo& pdo) override
    {
        messages_.push_back(pdo);
    }

    void end_cycle(bool valid) override
    {
        if (!valid || !publisher_)
            return;

        for (const auto& message : messages_)
            publisher_->publish(message);
    }

private:
    std::optional<ZenohPublisher> publisher_;
    std::vector<Pdo> messages_;
};

} 
