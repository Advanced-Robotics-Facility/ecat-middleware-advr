#include <dds/dds.hpp>

#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/service/service_client.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;

int main()
{
    advrf::log::Log::init();
    dds::domain::DomainParticipant participant(42);

    config::ConfigTopics topics({"advrf", "robot"});
    ServiceClient<RequestDDS, ResponseDDS> client(
        participant, topics.replCmd.request(), topics.replCmd.reply());

    RequestDDS request{};
    request.type() = static_cast<uint8_t>(4); // ECAT_MASTER_CMD, matching CmdType enum value
    request.ecat_master_cmd().type() = static_cast<uint8_t>(3); 
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG_INFO("Sending request...");
    auto response = client.call(request, std::chrono::milliseconds(2000));

    if (response) {
        LOG_INFO("Got reply. type={} msg={}", static_cast<int>(response->type()), response->msg());
    } else {
        LOG_WARN("No reply (timeout).");
    }

    return 0;
}