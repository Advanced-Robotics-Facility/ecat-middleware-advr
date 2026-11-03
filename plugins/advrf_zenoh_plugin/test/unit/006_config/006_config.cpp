#include <iostream>

#include <advrf_middleware_core/test/config_test_support.hpp>

#include "advrf_zenoh_plugin/config/plugin_config.hpp"

using namespace advrf::zenoh_plugin;
using namespace advrf::middleware::config;


int main() {
    try {
        const ConfigFile file{YAML::Load(R"(
                        robot: {name: kyon, joint_map: {1: knee}}
                        zenoh: {session_config: ../session.json5, wire_format: protobuf}
                        dds: {domain_id: garbage, qos: invalid}
                        )"), "/tmp/robot/middleware/config.yaml"};
        auto cfg = parse_plugin_config(file);
        require(cfg.transport.session_config == "/tmp/robot/session.json5", "relative session path");
        require(cfg.transport.wire_format == WireFormat::Protobuf, "wire format");
        expect_error([&] { validate(cfg); }, "zenoh.session_config");
        // Explicit CLI override replaces YAML value before validation.
        cfg.transport.session_config.clear();
        validate(cfg);
        require(cfg.common.runtime.rate_publishers == 1000, "common defaults");
        expect_error([] { parse_wire_format("invalid"); }, "zenoh.wire_format");
#ifndef ZENOH_ROS2_SUPPORT
        cfg.transport.wire_format = WireFormat::Ros2Cdr;
        expect_error([&] { validate(cfg); }, "ZENOH_ROS2_SUPPORT");
#else
        require(parse_wire_format("ros2cdr") == WireFormat::Ros2Cdr, "supported CDR format");
#endif
        std::cout << "Zenoh configuration tests passed\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
