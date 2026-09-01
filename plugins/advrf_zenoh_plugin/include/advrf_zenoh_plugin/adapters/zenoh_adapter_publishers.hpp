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
    : public advrf::middleware::adapters::message::AdapterPublishers
{
public:
    bool init(const advrf::middleware::config::ConfigTopics& topics,
              const advrf::middleware::config::RobotConfig& robot,
              const advrf::middleware::ecat::EcatDiscover::EcatMap& ecat_map,
              zenoh::Session& session,
              WireFormat wire_format);
};

} 
