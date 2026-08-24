#pragma once

#include <optional>
#include <string>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/publisher/zenoh_publisher.hpp"
#include "advrf_zenoh_plugin/config/wire_format.hpp"
#include "advrf_zenoh_plugin/serialization/serializer.hpp"

namespace advrf::zenoh_plugin
{

class ZenohAdapterBridgePublisher
    : public middleware_adapter::message::AdapterPublishers::IPublisher
{
public:
    using Pdo = iit::advrf::Ec_slave_pdo;

    bool init(zenoh::Session& session, 
              const std::string& topic_name,
              WireFormat wire_format,
              serialization::Ros2MessageType ros2_message_type)
    {
        try {
            topic_name_ = topic_name;
            wire_format_ = wire_format;

            publisher_.emplace(session, topic_name_);
            serializer_.emplace(wire_format_, ros2_message_type);

            LOG_INFO("Topic Created: {}", topic_name_);
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to declare Zenoh publisher '{}': {}",
                      topic_name_,
                      error.what()
            );
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
        if (!valid || !publisher_ || !serializer_ || messages_.empty())
            return;

        if (wire_format_ == WireFormat::Ros2Cdr) {
            std::vector<std::uint8_t> payload;

            if (!serializer_->serialize_cycle(messages_, payload)) {
                LOG_ERROR("Failed to serialize Zenoh topic '{}'.", topic_name_);
                return;
            }

            if (!publisher_->publish(std::move(payload), encoding_for(wire_format_))) {
                LOG_ERROR("Failed to write Zenoh topic '{}'.", topic_name_);
            }
            return;
        }

        for (const auto& message : messages_) {
            std::vector<std::uint8_t> payload;

            if (!serializer_->serialize(message, payload)) {
                LOG_ERROR("Failed to serialize Zenoh topic '{}'.", topic_name_);
                continue;
            }

            if (!publisher_->publish(std::move(payload), encoding_for(wire_format_))) {
                LOG_ERROR("Failed to write Zenoh topic '{}'.", topic_name_);
            }
        }
    }

private:
    std::optional<ZenohPublisher> publisher_;
    std::optional<serialization::Serializer> serializer_;

    std::vector<Pdo> messages_;

    std::string topic_name_;
    WireFormat wire_format_ = WireFormat::Protobuf;
};

}
