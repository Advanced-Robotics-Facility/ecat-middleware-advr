#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <zenoh.hxx>

#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/plugin/plugin_exec.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/adapters/zenoh_adapter_publishers.hpp"
#include "advrf_zenoh_plugin/adapters/zenoh_adapter_subscribers.hpp"
#include "advrf_zenoh_plugin/config/plugin_config.hpp"

//#include "advrf_zenoh_plugin/adapters/zenoh_adapter_services.hpp"

namespace {
using advrf::zenoh_plugin::WireFormat;
using namespace advrf::middleware::config;
struct Options {
    CommonOptions common;
    std::optional<WireFormat> wire_format;
    std::optional<std::string> zenoh_config;
};
Options parse_args(int argc, char** argv) {
    Options options;
    for (int index = 1; index < argc; ++index) {
        const std::string flag = argv[index];
        if (flag == "--help" || flag == "-h") { options.common.help = true; continue; }
        const auto text = argument_value(index, argc, argv);
        if (options.common.consume(flag, text)) continue;
        if (flag == "--wire-format") options.wire_format = advrf::zenoh_plugin::parse_wire_format(text);
        else if (flag == "--zenoh-config") options.zenoh_config = text;
        else throw ConfigError("Unknown argument: " + flag);
    }
    return options;
}
zenoh::Config make_zenoh_config(const advrf::zenoh_plugin::ZenohConfig& config) {
    if (!config.session_config.empty()) return zenoh::Config::from_file(config.session_config.string());
    if (std::getenv("ZENOH_CONFIG") != nullptr) return zenoh::Config::from_env();
    return zenoh::Config::create_default();
}
}

int main(int argc, char** argv) {
    advrf::log::Log::init();
    using namespace advrf::middleware::config;
    try {
        const auto options = parse_args(argc, argv);
        if (options.common.help) {
            std::cout << "Usage: zenoh_plugin [OPTIONS]\n" << common_options_help()
                      << "  --wire-format <FORMAT>  protobuf or ros2cdr\n"
                      << "  --zenoh-config <PATH>   Override Zenoh JSON5 configuration\n";
            return 0;
        }
        const auto path = options.common.config_path.empty()
            ? ADVRF_CONFIG_SHARE / "middleware" / "config.yaml" : options.common.config_path;
        const auto file = ConfigFile::load(path);
        auto config = advrf::zenoh_plugin::parse_plugin_config(file);
        options.common.runtime.apply(config.common.runtime);
        if (options.wire_format) config.transport.wire_format = *options.wire_format;
        // CLI paths are relative to the working directory; YAML references to the YAML file.
        if (options.zenoh_config) config.transport.session_config = *options.zenoh_config;
        advrf::zenoh_plugin::validate(config);
        zenoh::init_log_from_env_or("error");
        auto session_config = make_zenoh_config(config.transport);
        const auto& robot = config.common.robot;
        const auto& runtime = config.common.runtime;
        ConfigTopics topics({robot.ns, robot.robot_name});

        advrf::middleware::ecat::EcatDiscover discover;
        if (!discover.start(SHM_NRT_RX_PDO)) {
            LOG_ERROR("Failed to connect to EtherCAT PDO shared memory.");
            return 1;
        }
        const auto ecat_map = discover.discover(extract_pdo_ids(robot));
        auto session = zenoh::Session::open(std::move(session_config));
        auto publishers = std::make_shared<advrf::zenoh_plugin::ZenohAdapterPublishers>();
        if (!publishers->init(topics, robot, ecat_map, session, config.transport.wire_format))
            throw std::runtime_error("Failed to initialize Zenoh publishers");
        auto subscribers = std::make_shared<advrf::zenoh_plugin::ZenohAdapterSubscribers>(
            topics, session, config.transport.wire_format);

        advrf::middleware::plugin::PluginExec exec;
        exec.register_adapter({"zenoh_adapter_publishers", publishers, period_from_rate(runtime.rate_publishers)});
        exec.register_adapter({"zenoh_adapter_subscribers", subscribers, period_from_rate(runtime.rate_subscribers)});
        LOG_INFO("Configuration: {}; publishers: {} Hz; subscribers: {} Hz; service: {} Hz",
                 file.path.string(), runtime.rate_publishers, runtime.rate_subscribers, runtime.rate_service);
        exec.start();
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return 1;
    }
}
