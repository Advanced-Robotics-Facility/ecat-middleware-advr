#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <zenoh.hxx>

#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::zenoh_plugin
{

class ZenohPublisher
{
public:
    ZenohPublisher(
        zenoh::Session& session,
        const std::string& topic_name,
        const zenoh::Encoding& encoding)
            : publisher_(declare(session, topic_name, encoding))
    {

    }

    bool publish(std::vector<std::uint8_t> payload)
    {
        try {
            publisher_.put(zenoh::Bytes(std::move(payload)));
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to publish Zenoh topic_name '{}': {}", publisher_.get_keyexpr().as_string_view(), error.what());
            return false;
        }
    }

private:
    static zenoh::Publisher declare(
        zenoh::Session& session,
        const std::string& topic_name,
        const zenoh::Encoding& encoding)
    {
        zenoh::Session::PublisherOptions options;
        options.congestion_control = Z_CONGESTION_CONTROL_DROP;
        options.priority = Z_PRIORITY_REAL_TIME;
        options.is_express = true;
        options.encoding = encoding;

        return session.declare_publisher(
            zenoh::KeyExpr(topic_name),
            std::move(options)
        );
    }

    zenoh::Publisher publisher_;
};

}
