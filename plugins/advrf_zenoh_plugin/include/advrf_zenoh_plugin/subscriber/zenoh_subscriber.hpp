#pragma once

#include <deque>
#include <functional>
#include <mutex>
#include <string>

#include <zenoh.hxx>

#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/serialization/protobuf.hpp"

namespace advrf::zenoh_plugin
{

class IZenohSubscriber
{
public:
    virtual ~IZenohSubscriber() = default;
    virtual void spin_once() = 0;
};

template<typename Message>
class ZenohSubscriber final : public IZenohSubscriber
{
public:
    using Callback = std::function<void(const Message&)>;
    using Deserializer =
        std::function<bool(const std::vector<std::uint8_t>&, Message&)>;

    ZenohSubscriber(zenoh::Session& session,
                    const std::string& key,
                    Callback callback,
                    Deserializer deserializer =
                        [](const std::vector<std::uint8_t>& payload,
                           Message& message)
                        {
                            const deserialization::ProtobufDeserializer decoder;
                            return decoder.deserialize(payload, message);
                        })
        : callback_(std::move(callback))
        , deserializer_(std::move(deserializer))
        , subscriber_(session.declare_subscriber(
            zenoh::KeyExpr(key),
            [this, key](zenoh::Sample& sample)
            {
                Message message;
                const auto payload = sample.get_payload().as_vector();

                if (!deserializer_(payload, message))
                {
                    LOG_ERROR("Failed to deserialize Protobuf payload for Zenoh key '{}'.", key);
                    return;
                }

                std::lock_guard<std::mutex> lock(mutex_);
                pending_.emplace_back(std::move(message));
            },
            zenoh::closures::none))
    {}

    void spin_once() override
    {
        std::deque<Message> pending;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending.swap(pending_);
        }

        for (const auto& message : pending)
        {
            if (!callback_)
                continue;

            try
            {
                callback_(message);
            }
            catch (const std::exception& error)
            {
                LOG_ERROR("Zenoh subscriber callback failed: {}", error.what());
            }
            catch (...)
            {
                LOG_ERROR("Zenoh subscriber callback failed with an unknown error.");
            }
        }
    }

private:
    Callback callback_;
    Deserializer deserializer_;
    std::mutex mutex_;
    std::deque<Message> pending_;
    zenoh::Subscriber<void> subscriber_;
};

}
