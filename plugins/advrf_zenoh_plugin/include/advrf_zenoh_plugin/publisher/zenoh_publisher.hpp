#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <zenoh.hxx>

#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::zenoh_plugin
{

class ZenohPublisher
{
public:
    ZenohPublisher(zenoh::Session& session, const std::string& topic_name)
        : publisher_(session.declare_publisher(zenoh::KeyExpr(topic_name)))
    {}

    bool publish(std::vector<std::uint8_t> payload,
                 const zenoh::Encoding& encoding)
    {
        try {
            zenoh::Publisher::PutOptions options;
            options.encoding = encoding;
            publisher_.put(zenoh::Bytes(std::move(payload)), std::move(options));
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to publish Zenoh topic_name '{}': {}", publisher_.get_keyexpr().as_string_view(), error.what());
            return false;
        }
    }

private:
    zenoh::Publisher publisher_;
};

}
