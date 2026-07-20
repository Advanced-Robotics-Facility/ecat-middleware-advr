#include <iostream>
#include <dds/dds.hpp>

#include "advrf_cyclonedds_plugin/service/service_client.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"

#include <advrf_interfaces/srv/ReplCmd.hpp>

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;

int main()
{
    dds::domain::DomainParticipant participant(42);

    config::ConfigTopics topics({"advrf", "robot"});
    ServiceClient<RequestDDS, ResponseDDS> client(
        participant, topics.replCmd.request(), topics.replCmd.reply());

    RequestDDS request{};
    request.type() = static_cast<uint8_t>(4); // ECAT_MASTER_CMD, matching CmdType enum value
    request.ecat_master_cmd().type() = static_cast<uint8_t>(3); 
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Sending request..." << std::endl;
    auto response = client.call(request, std::chrono::milliseconds(2000));

    if (response) {
        std::cout << "Got reply. type=" << static_cast<int>(response->type())
                  << " msg=" << response->msg() << std::endl;
    } else {
        std::cout << "No reply (timeout)." << std::endl;
    }

    return 0;
}