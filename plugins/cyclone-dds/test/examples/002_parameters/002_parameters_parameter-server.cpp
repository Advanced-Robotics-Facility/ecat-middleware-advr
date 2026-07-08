#include <dds/dds.hpp>
#include <iostream>
#include "parameter/parameter_server.hpp"

constexpr int DOMAIN_ID = 42;

int main()
{
    dds::domain::DomainParticipant participant(DOMAIN_ID);
    
    config::ConfigTopics config_topics({"advrf", "robot"});
    ParameterServer server(config_topics, participant);
    
    server.registry().declare(
        "gains/kp",
        12.0,
        {
            .description = "Proportional gain"
        });

    server.registry().declare(
        "gains/ki",
        0.2,
        {
            .description = "Integral gain"
        });

    server.registry().declare(
        "gains/kd",
        0.05,
        {
            .description = "Derivative gain"
        });

    server.registry().declare(
        "imu/enabled",
        true);

    server.registry().declare(
        "robot/name",
        std::string("Spot"));

    std::cout << "Parameter server ready." << std::endl;

    server.spin();
}