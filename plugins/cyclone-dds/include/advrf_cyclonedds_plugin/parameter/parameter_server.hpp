#pragma once

#include <chrono>
#include <thread>
#include <dds/dds.hpp>

#include <advrf_interfaces/srv/GetParameters.hpp>
#include <advrf_interfaces/srv/ListParameters.hpp>
#include <advrf_interfaces/srv/ListGetParameters.hpp>
#include <advrf_interfaces/srv/SetParameters.hpp>
#include <rcl_interfaces/msg/RequestHeader.hpp>

#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/parameter/parameter_registry.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/service.hpp"

class ParameterServer
{
public:

    explicit ParameterServer(const config::ConfigTopics& config_topics,
        dds::domain::DomainParticipant& participant);

    ParameterRegistry&
    registry()
    {
        return registry_;
    }

    const ParameterRegistry&
    registry() const
    {
        return registry_;
    }

    void spin_once();

    void spin(
        std::chrono::milliseconds period =
            std::chrono::milliseconds(1));

private:

    ParameterRegistry registry_;

    ServiceServer<
        advrf_interfaces::srv::dds_::GetParameters_Request_,
        advrf_interfaces::srv::dds_::GetParameters_Response_> get_server_;

    ServiceServer<
        advrf_interfaces::srv::dds_::SetParameters_Request_,
        advrf_interfaces::srv::dds_::SetParameters_Response_> set_server_;

    ServiceServer<
        advrf_interfaces::srv::dds_::ListParameters_Request_,
        advrf_interfaces::srv::dds_::ListParameters_Response_> list_server_;

    ServiceServer<
        advrf_interfaces::srv::dds_::ListGetParameters_Request_,
        advrf_interfaces::srv::dds_::ListGetParameters_Response_> listget_server_;
};