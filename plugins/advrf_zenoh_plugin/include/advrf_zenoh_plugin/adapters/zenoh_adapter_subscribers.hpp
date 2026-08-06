#pragma once

#include <string>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>

#include "advrf_zenoh_plugin/subscriber/zenoh_subscriber.hpp"

namespace advrf::zenoh_plugin
{

class ZenohAdapterSubscribers
    : public middleware_adapter::message::AdapterSubscribers
{
public:
    ZenohAdapterSubscribers(const config::ConfigTopics& topics,
                            zenoh::Session& session);

    bool start() override;
    void spin_once() override {}

private:
    using Commands = iit::advrf::Repl_cmd_vector;

    zenoh::Session& session_;
    std::string key_;
    std::optional<ZenohSubscriber<Commands>> subscriber_;
};

}
