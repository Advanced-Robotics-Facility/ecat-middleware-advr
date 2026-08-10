#include <chrono>
#include <iostream>
#include <string>

#include <advrf_middleware_core/plugin/plugin_exec.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"

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
                "  advrf_cyclonedds_plugin [OPTIONS]\n\n"
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

    auto dds_participant = dds::domain::DomainParticipant(cfg->domain_id);
    auto config = config::ConfigTopics({"advrf", cfg->robot_name});

    // service
    //auto dds_adapter_service = std::make_shared<DDSAdapterService>(config, dds_participant);

    // publishers
    auto dds_adapter_publishers = std::make_shared<DDSAdapterPublishers>();
    if (!dds_adapter_publishers->init(config, *cfg, dds_participant))
    {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    // subscribers
    auto dds_adapter_subscribers = std::make_shared<DDSAdapterSubscribers>(config, dds_participant);


    // plugin_exec.register_adapter({
    //     "dds_adapter_service",
    //     dds_adapter_service,
    //     period_from_rate(options.rate_service)
    // });

    plugin_exec.register_adapter({
        "dds_adapter_publishers",
        dds_adapter_publishers,
        period_from_rate(options.rate_publishers)
    });

    plugin_exec.register_adapter({
        "dds_adapter_subscribers",
        dds_adapter_subscribers,
        period_from_rate(options.rate_subscribers)
    });

    plugin_exec.start();

    return 0;
}