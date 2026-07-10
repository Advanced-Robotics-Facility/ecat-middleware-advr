#include <iostream>
#include <dds/dds.hpp>


#include "advrf_cyclonedds_plugin/parameter/parameter_client.hpp"

constexpr int DOMAIN_ID = 42;

int main()
{
    dds::domain::DomainParticipant participant(DOMAIN_ID);
    
    config::ConfigTopics config_topics({"advrf", "robot"});
    ParameterClient client(config_topics, participant);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //
    // List
    //

    std::cout << "==== PARAMETERS ====\n";

    for (const auto& name : client.list())
    {
        std::cout << " - " << name << '\n';
    }

    //
    // Read
    //

    auto kp = client.get<double>("gains/kp");
    auto enabled = client.get<bool>("imu/enabled");

    std::cout << "\nkp      = " << kp << '\n';
    std::cout << "enabled = " << std::boolalpha << enabled << '\n';

    //
    // Write
    //

    client.set("gains/kp", 25.0);
    client.set("imu/enabled", false);

    std::cout << "\nUpdated values\n";

    std::cout
        << "kp = "
        << client.get<double>("gains/kp")
        << '\n';

    std::cout
        << "enabled = "
        << std::boolalpha
        << client.get<bool>("imu/enabled")
        << '\n';

    //
    // Read all
    //

    std::cout << "\n==== ALL PARAMETERS ====\n";

    for (const auto& parameter : client.get())
    {
        std::cout << parameter.name() << '\n';
    }

    //
    // Prefix
    //

    std::cout << "\n==== GAINS ====\n";

    for (const auto& name : client.list("gains"))
    {
        std::cout << name << '\n';
    }
}