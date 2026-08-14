#pragma once

#include <optional>
#include <string>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/publisher/zenoh_publisher.hpp"

namespace advrf::zenoh_plugin
{

class ZenohAdapterBridgePublisher
    : public middleware_adapter::message::AdapterPublishers::IPublisher
{
public:
    using Pdo = iit::advrf::Ec_slave_pdo;

    bool init(zenoh::Session& session, const std::string& topic_name)
    {
        try {
            topic_name_ = topic_name;
            publisher_.emplace(session, topic_name_);
            LOG_INFO("Topic Created: {}", topic_name_);
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to declare Zenoh publisher '{}': {}",
                      topic_name_,
                      error.what());
            return false;
        }
    }

    const std::string& topic_name() const
    {
        return topic_name_;
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

        for (const auto& message : messages_) {
            if (!publisher_->publish(message)) {
                LOG_ERROR("Failed to write Zenoh topic '{}'.", topic_name_);
            }
        }
    }

private:
    std::optional<ZenohPublisher> publisher_;
    std::vector<Pdo> messages_;
    std::string topic_name_;
};

}
