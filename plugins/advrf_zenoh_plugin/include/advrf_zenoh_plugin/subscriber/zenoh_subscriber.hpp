#pragma once

#include <functional>
#include <string>

#include <zenoh.hxx>

#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/serialization/protobuf.hpp"

namespace advrf::zenoh_plugin
{

template<typename Message>
class ZenohSubscriber
{
public:
    using Callback = std::function<void(const Message&)>;

    ZenohSubscriber(zenoh::Session& session,
                    const std::string& key,
                    Callback callback)
        : subscriber_(session.declare_subscriber(
            zenoh::KeyExpr(key),
            [key, callback = std::move(callback)](zenoh::Sample& sample)
                mutable
            {
                Message message;
                const auto payload = sample.get_payload().as_vector();

                if (!protobuf::deserialize(payload, message))
                {
                    LOG_ERROR("Failed to deserialize Protobuf payload for Zenoh key '{}'.", key);
                    return;
                }

                if (!callback)
                    return;

                try {
                    callback(message);
                } catch (const std::exception& error) {
                    LOG_ERROR("Zenoh subscriber callback for key '{}' failed: {}", key, error.what());
                } catch (...) {
                    LOG_ERROR("Zenoh subscriber callback for key '{}' failed with an unknown error.", key);
                }
            },
            zenoh::closures::none))
    {}

private:
    zenoh::Subscriber<void> subscriber_;
};

} 
