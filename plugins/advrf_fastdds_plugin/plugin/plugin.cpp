#include <chrono>
#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <advrf_middleware_core/plugin/plugin_exec.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_publishers.hpp"
//#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
//#include "advrf_fastdds_plugin/adapters/dds_adapter_service.hpp"

namespace
{
struct Options
{
    uint32_t rate_publishers = 1000; // Hz
    uint32_t rate_service    = 10;   // Hz
    uint32_t rate_subscribers = 100; // Hz
};

Options parse_args(int argc, char **argv)
{
    Options opt;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "--rate_publishers" && i + 1 < argc)
        {
            opt.rate_publishers = std::stoul(argv[++i]);
        }
        else if (arg == "--rate_service" && i + 1 < argc)
        {
            opt.rate_service = std::stoul(argv[++i]);
        }
        else if (arg == "--rate_subscribers" && i + 1 < argc)
        {
            opt.rate_subscribers = std::stoul(argv[++i]);
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout <<
                "Usage:\n"
                "  advrf_fastdds_plugin [OPTIONS]\n\n"
                "Options:\n"
                "  --rate_publishers <Hz>   Publisher frequency (default: 1000)\n"
                "  --rate_service <Hz>      Service frequency (default: 10)\n"
                "  --rate_subscribers <Hz>  Subscribers frequency (default: 100)\n";
            std::exit(0);
        }
    }
    return opt;
}

std::chrono::microseconds period_from_rate(uint32_t hz)
{
    if (hz == 0)
        throw std::runtime_error("Rate must be > 0");

    return std::chrono::microseconds(1'000'000 / hz);
}
} // namespace

int main(int argc, char **argv)
{
    advrf::log::Log::init();

    const auto options = parse_args(argc, argv);

    advrf::plugin::PluginExec plugin_exec;

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg)
        return 1;

    auto* dds_participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        cfg->domain_id,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (dds_participant == nullptr) {
        LOG_ERROR("Failed to create DDS DomainParticipant.");
        return 1;
    }

    auto config = config::ConfigTopics({"advrf", cfg->robot_name});

    // service
    //auto dds_adapter_service = std::make_shared<DDSAdapterService>(config, dds_participant);

    // publishers
    auto dds_adapter_publishers = std::make_shared<DDSAdapterPublishers>();
    
    // subscribers
    //auto dds_adapter_subscribers = std::make_shared<DDSAdapterSubscribers>(config, dds_participant);
   
    if (!dds_adapter_publishers->init(config, *cfg, dds_participant))
    {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    // plugin_exec.register_adapter({
    //     dds_adapter_service,
    //     period_from_rate(options.rate_service)
    // });
    // LOG_INFO("Service registered");

    plugin_exec.register_adapter({
        dds_adapter_publishers,
        period_from_rate(options.rate_publishers)
    });
    LOG_INFO("Publishers registered");

    // plugin_exec.register_adapter({
    //     dds_adapter_subscribers,
    //     period_from_rate(options.rate_subscribers)
    // });
    // LOG_INFO("Subscribers registered");


    LOG_INFO("Publisher rate: {} Hz", options.rate_publishers);
    // LOG_INFO("Service rate: {} Hz", options.rate_service);
    // LOG_INFO("Subscribers rate: {} Hz", options.rate_subscribers);

    plugin_exec.start();

    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dds_participant);

    return 0;
}