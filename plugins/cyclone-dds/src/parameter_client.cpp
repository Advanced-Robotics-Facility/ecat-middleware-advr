#include "advrf_cyclonedds_plugin/parameter/parameter_client.hpp"

#include <stdexcept>

using namespace advrf_interfaces::srv::dds_;
using namespace rcl_interfaces::msg::dds_;

ParameterClient::ParameterClient(const config::ConfigTopics& config_topics,
    dds::domain::DomainParticipant& participant)
    : get_client_(
        participant,
        config_topics.parameters.getRequest(),
        config_topics.parameters.getReply())
    , set_client_(
        participant,
        config_topics.parameters.setRequest(),
        config_topics.parameters.setReply())
    , list_client_(
        participant,
        config_topics.parameters.listRequest(),
        config_topics.parameters.listReply()),
    listget_client_(
        participant,
        config_topics.parameters.listGetRequest(),
        config_topics.parameters.listGetReply())
{
}

std::vector<Parameter_>
ParameterClient::get()
{
    GetParameters_Request_ request;

    auto response = get_client_.call(request);

    if (!response)
        throw std::runtime_error("GetParameters service timeout.");

    if (!response->success())
        throw std::runtime_error("GetParameters service failed.");

    return response->parameters();
}

std::vector<Parameter_>
ParameterClient::get(
    const std::vector<std::string>& names)
{
    GetParameters_Request_ request;
    request.names(names);
    auto response = get_client_.call(request);
    if (!response)
        throw std::runtime_error("GetParameters service timeout.");

    if (!response->success())
        throw std::runtime_error("GetParameters service failed.");

    return response->parameters();
}

std::vector<ParameterResult>
ParameterClient::set(
    const std::vector<Parameter_>& parameters)
{
    SetParameters_Request_ request;

    request.parameters(parameters);

    auto response = set_client_.call(request);

    if (!response)
        throw std::runtime_error("SetParameters service timeout.");

    if (!response->success())
        throw std::runtime_error("SetParameters service failed.");

    std::vector<ParameterResult> results;
    results.reserve(response->results().size());

    for (auto result : response->results())
    {
        results.emplace_back(
            static_cast<ParameterResult>(result));
    }

    return results;
}

std::vector<std::string>
ParameterClient::list()
{
    return list("");
}

std::vector<std::string>
ParameterClient::list(
    const std::string& prefix)
{
    ListParameters_Request_ request;

    request.prefix(prefix);

    auto response = list_client_.call(request);

    if (!response)
        throw std::runtime_error("ListParameters service timeout.");

    if (!response->success())
        throw std::runtime_error("ListParameters service failed.");

    return response->names();
}


std::vector<Parameter_>ParameterClient::listGet(const std::string& prefix)
{
    ListGetParameters_Request_ request;
    request.prefix(prefix);

    auto response = listget_client_.call(request);
    if (!response)
        throw std::runtime_error("GetParameters service timeout.");

    if (!response->success())
        throw std::runtime_error("GetParameters service failed.");

    return response->parameters();
}