#include "advrf_zenoh_plugin/adapters/zenoh_adapter_subscribers.hpp"

#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::zenoh_plugin
{

ZenohAdapterSubscribers::ZenohAdapterSubscribers(
        const config::ConfigTopics& topics,
        zenoh::Session& session)
    : session_(session)
    , key_(topics.command.jointCmd())
{}

bool ZenohAdapterSubscribers::start()
{
    if (!middleware_adapter::message::AdapterSubscribers::start())
        return false;

    try
    {
        subscriber_.emplace(
            session_,
            key_,
            [this](const Commands& commands) { forward(commands); });
        return true;
    }
    catch (const zenoh::ZException& error)
    {
        LOG_ERROR("Failed to declare Zenoh subscriber '{}': {}", key_, error.what());
        return false;
    }
}

}
