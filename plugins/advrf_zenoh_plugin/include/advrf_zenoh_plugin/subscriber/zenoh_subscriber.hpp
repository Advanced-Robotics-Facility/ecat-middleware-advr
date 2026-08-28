#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <utility>

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
                        [](const std::vector<std::uint8_t>& payload, Message& message)
                        {
                            const deserialization::ProtobufDeserializer decoder;
                            return decoder.deserialize(payload, message);
                        },
                    std::size_t history_depth = 1)
        : callback_(std::move(callback))
        , deserializer_(std::move(deserializer))
        , history_depth_(std::max<std::size_t>(1, history_depth))
        , subscriber_(session.declare_subscriber(
            zenoh::KeyExpr(key),
            [this, key](zenoh::Sample& sample)
            {
                Message message;
                const auto payload = sample.get_payload().as_vector();

                if (!deserializer_(payload, message))
                {
                    LOG_ERROR("Failed to deserialize payload for Zenoh key '{}'.", key);
                    return;
                }

                std::lock_guard<std::mutex> lock(mutex_);

                while (pending_.size() >= history_depth_)
                {
                    pending_.pop_front();
                    ++dropped_samples_;
                }

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
    std::size_t history_depth_{1};
    std::uint64_t dropped_samples_{0};
    std::mutex mutex_;
    std::deque<Message> pending_;
    zenoh::Subscriber<void> subscriber_;
};

}
