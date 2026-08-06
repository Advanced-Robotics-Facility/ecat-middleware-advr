#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <zenoh.hxx>

#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/serialization/protobuf.hpp"

namespace advrf::zenoh_plugin
{

class ZenohPublisher
{
public:
    ZenohPublisher(zenoh::Session& session, const std::string& key)
        : publisher_(session.declare_publisher(zenoh::KeyExpr(key)))
    {}

    template<typename Message>
    bool publish(const Message& message)
    {
        std::vector<std::uint8_t> payload;
        if (!protobuf::serialize(message, payload)) {
            LOG_ERROR("Failed to serialize Protobuf payload for Zenoh key '{}'.", publisher_.get_keyexpr().as_string_view());
            return false;
        }

        try {
            publisher_.put(zenoh::Bytes(std::move(payload)));
            return true;
        } catch (const zenoh::ZException& error) {
            LOG_ERROR("Failed to publish Zenoh key '{}': {}", publisher_.get_keyexpr().as_string_view(), error.what());
            return false;
        }
    }

private:
    zenoh::Publisher publisher_;
};

}
