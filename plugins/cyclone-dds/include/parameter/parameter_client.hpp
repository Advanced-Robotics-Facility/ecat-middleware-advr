#pragma once

#include <string>
#include <vector>

#include <dds/dds.hpp>

#include "advrf_interfaces/srv/GetParameters.hpp"
#include "advrf_interfaces/srv/ListParameters.hpp"
#include "advrf_interfaces/srv/SetParameters.hpp"
#include "advrf_interfaces/srv/ListGetParameters.hpp"
#include "advrf_interfaces/msg/Enums.hpp"

#include "config/config_topics.hpp"
#include "parameter/parameter_make_get.hpp"
#include "service.hpp"


using ParameterResult = advrf_interfaces::msg::dds_::enums_::ParameterResult_;


/**
 * @brief Client for interacting with parameters.
 */
class ParameterClient
{
public:

    /**
     * @brief Construct a new Parameter Client object.
     *
     * @param participant The DDS domain participant.
     */
    explicit ParameterClient(const config::ConfigTopics& config_topics, dds::domain::DomainParticipant& participant);

    template<class T>
    T get(const std::string& name);

    std::vector<rcl_interfaces::msg::dds_::Parameter_> get();

    std::vector<rcl_interfaces::msg::dds_::Parameter_>
    get(const std::vector<std::string>& names);

    template<class T>
    ParameterResult set(
        const std::string& name,
        const T& value);

    std::vector<ParameterResult> set(const std::vector<rcl_interfaces::msg::dds_::Parameter_>& parameters);
    std::vector<std::string> list();
    std::vector<std::string> list(const std::string& prefix);
    std::vector<Parameter_> listGet(const std::string& prefix);

private:

    ServiceClient<
        advrf_interfaces::srv::dds_::GetParameters_Request_,
        advrf_interfaces::srv::dds_::GetParameters_Response_> get_client_;

    ServiceClient<
        advrf_interfaces::srv::dds_::SetParameters_Request_,
        advrf_interfaces::srv::dds_::SetParameters_Response_> set_client_;

    ServiceClient<
        advrf_interfaces::srv::dds_::ListParameters_Request_,
        advrf_interfaces::srv::dds_::ListParameters_Response_> list_client_;

    ServiceClient<
        advrf_interfaces::srv::dds_::ListGetParameters_Request_,
        advrf_interfaces::srv::dds_::ListGetParameters_Response_> listget_client_;
};


template<class T>
T ParameterClient::get(const std::string& name)
{
    auto parameters = get(std::vector<std::string>{name});

    if (parameters.empty())
        throw std::runtime_error("Parameter '" + name + "' not found.");

    return get_parameter<T>(parameters.front());
}

template<class T>
ParameterResult ParameterClient::set(
    const std::string& name,
    const T& value)
{
    auto results = set({
        make_parameter(name, value)
    });

    if (results.empty())
        return ParameterResult::NotFound;

    return results.front();
}