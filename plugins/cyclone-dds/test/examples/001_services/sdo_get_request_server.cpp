#include <dds/dds.hpp>

#include "parameter.hpp"
#include "service.hpp"

#include <advrf_interfaces/srv/SdoGetRequest.hpp>

constexpr int DOMAIN_ID = 42;

int main()
{
    dds::domain::DomainParticipant participant(DOMAIN_ID);

    ServiceServer<
        std_srvs::srv::dds_::SdoGet_Request_,
        std_srvs::srv::dds_::SdoGet_Response_> server(
            participant,
            "rq/advrf/spot/sdoGetRequest",
            "rr/advrf/spot/sdoGetReply");

    server.set_callback([](const auto& request)
    {
        std_srvs::srv::dds_::SdoGet_Response_ response;

        response.success(true);

        std::vector<rcl_interfaces::msg::dds_::Parameter_> parameters;

        switch (request.sub_request())
        {
            case std_srvs::srv::dds_::SUB_REQUEST_ALL:
            {
                parameters.emplace_back(make_parameter("kp", 12.5));
                parameters.emplace_back(make_parameter("ki", 0.3));
                parameters.emplace_back(make_parameter("kd", 0.02));
                parameters.emplace_back(make_parameter("enabled", true));
                parameters.emplace_back(make_parameter("robot_name", std::string("Spot")));
                parameters.emplace_back(make_parameter("ids", std::vector<int64_t>{1,2,3,4}));
                break;
            }

            case std_srvs::srv::dds_::SUB_REQUEST_GAINS:
            {
                parameters.emplace_back(make_parameter("kp", 12.5));
                parameters.emplace_back(make_parameter("ki", 0.3));
                parameters.emplace_back(make_parameter("kd", 0.02));
                break;
            }

            default:
            {
                response.success(false);
                return response;
            }
        }

        response.parameters(parameters);

        return response;
    });

    std::cout << "Server ready." << std::endl;

    server.spin();
}