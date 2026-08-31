#pragma once

#include <zenoh.hxx>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>

#include "advrf_zenoh_plugin/config/wire_format.hpp"

namespace advrf::zenoh_plugin
{

class ZenohAdapterPublishers
    : public middleware_adapter::message::AdapterPublishers
{
public:
    bool init(const config::ConfigTopics& topics,
              const config::RobotConfig& robot,
              const EcatDiscover::EcatMap& ecat_map,
              zenoh::Session& session,
              WireFormat wire_format);
};

} 
