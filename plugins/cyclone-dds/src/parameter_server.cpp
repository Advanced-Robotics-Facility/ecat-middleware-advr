#include "parameter/parameter_server.hpp"
#include "config/config_topics.hpp"

using namespace advrf_interfaces::srv::dds_;

ParameterServer::ParameterServer(const config::ConfigTopics& config_topics,
    dds::domain::DomainParticipant& participant)
    : get_server_(
      participant,
      config_topics.parameters.getRequest(),
      config_topics.parameters.getReply())
, set_server_(
      participant,
      config_topics.parameters.setRequest(),
      config_topics.parameters.setReply())
, list_server_(
      participant,
      config_topics.parameters.listRequest(),
      config_topics.parameters.listReply())
{
    //
    // GetParameters
    //

    get_server_.set_callback(
        [this](const GetParameters_Request_& request)
        {
            GetParameters_Response_ response;
            response.request_id(request.request_id());
            response.success(true);
            if (request.names().empty())
            {
                response.parameters(registry_.get());
            }
            else
            {
                response.parameters(
                    registry_.get(request.names()));
            }
            return response;
        });

    //
    // SetParameters
    //

    set_server_.set_callback(
        [this](const SetParameters_Request_& request)
        {
            SetParameters_Response_ response;
            response.request_id(request.request_id());

            auto results = registry_.set(request.parameters());
            response.success(true);

            std::vector<uint8_t> dds_results;
            dds_results.reserve(results.size());

            for (auto result : results)
            {
                dds_results.emplace_back(
                    static_cast<uint8_t>(result));
            }

            response.results(dds_results);
            return response;
        });

    //
    // ListParameters
    //

    list_server_.set_callback(
        [this](const ListParameters_Request_& request)
        {
            ListParameters_Response_ response;
            response.request_id(request.request_id());
            response.success(true);
            if (request.prefix().empty())
            {
                response.names(registry_.list());
            }
            else
            {
                response.names(
                    registry_.list(request.prefix()));
            }
            return response;
        });
}

void ParameterServer::spin_once()
{
    get_server_.spin_once();
    set_server_.spin_once();
    list_server_.spin_once();
}

void ParameterServer::spin(
    std::chrono::milliseconds period)
{
    while (true)
    {
        spin_once();
        std::this_thread::sleep_for(period);
    }
}