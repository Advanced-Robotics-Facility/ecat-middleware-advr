#include <dds/dds.hpp>

#include "advrf_cyclonedds_plugin/service/service_server.hpp"
#include "advrf_interfaces/srv/TriggerRequest.hpp"

constexpr int DOMAIN_ID = 42;

int main()
{
    dds::domain::DomainParticipant participant(DOMAIN_ID);

    ServiceServerNotSequential<
        advrf_interfaces::srv::dds_::Trigger_Request_,
        advrf_interfaces::srv::dds_::Trigger_Response_> server(
            participant,
            "rq/advrf/spot/triggerRequest",
            "rr/advrf/spot/triggerReply");

    // server.set_callback([](const auto& request)
    // {
        
    // });
}