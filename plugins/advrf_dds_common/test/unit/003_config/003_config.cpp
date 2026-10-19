#include <iostream>

#include <advrf_middleware_core/test/config_test_support.hpp>

#include "advrf_dds_common/config/dds_config.hpp"

using namespace advrf::dds_common;
using namespace advrf::middleware::config;
int main() {
    try {
        const auto root = YAML::Load(R"(
robot: {name: kyon, namespace: advrf, joint_map: {1: knee}}
dds:
  domain_id: 42
  declare_to_ros: true
  qos:
    state: {reliability: reliable, durability: transient_local, depth: 8}
    command: {reliability: best_effort, durability: volatile, depth: 2}
)");
        const auto cfg = parse_dds_plugin_config({root, "/tmp/config.yaml"});
        validate(cfg);
        require(cfg.dds.domain_id == 42 && cfg.dds.declare_to_ros, "DDS settings");
        require(cfg.dds.state.reliability == Reliability::Reliable && cfg.dds.state.durability == Durability::TransientLocal && cfg.dds.state.history_depth == 8,
                "state QoS");
        require(cfg.dds.command.reliability == Reliability::BestEffort && cfg.dds.command.durability == Durability::Volatile && cfg.dds.command.history_depth == 2,
                "independent command QoS");
        const auto legacy = parse_dds_config(YAML::Load("dds: {domain: 23, qos: {reliability: reliable, durability: transient_local, history: keep_last, depth: 3}}"));
        require(legacy.domain_id == 23 && legacy.state.history_depth == 3 && legacy.command.history_depth == 3, "legacy flat QoS");
        const auto defaults = parse_dds_config(YAML::Load("{}"));
        require(defaults.state.reliability == Reliability::BestEffort && defaults.command.history_depth == 1, "defaults");
        using advrf::middleware::adapters::service::ServiceTransport;
        require(defaults.service.transport == ServiceTransport::Shm, "default SHM service");
        const auto zmq = parse_dds_config(YAML::Load("dds: {service: {transport: zmq, command_uri: 'tcp://127.0.0.1:7777', timeout_ms: 250}}"));
        require(zmq.service.transport == ServiceTransport::Zmq && zmq.service.timeout_ms == 250 &&
                zmq.service.command_uri == "tcp://127.0.0.1:7777", "ZMQ service selection");
        expect_error([] { parse_dds_config(YAML::Load("dds: {service: {transport: invalid}}")); }, "dds.service.transport");
        expect_error([] { parse_dds_config(YAML::Load("dds: {service: {timeout_ms: 0}}")); }, "dds.service");
        expect_error([] { parse_dds_config(YAML::Load("dds: {qos: {state: {depth: 0}}}")); }, "dds.qos.state.depth");
        expect_error([] { parse_dds_config(YAML::Load("dds: {qos: {command: {depth: 2147483648}}}")); }, "dds.qos.command.depth");
        expect_error([] { parse_dds_config(YAML::Load("dds: {qos: {state: {history: keep_all}}}")); }, "dds.qos.state.history");
        expect_error([] { parse_dds_config(YAML::Load("dds: {qos: {command: {reliability: invalid}}}")); }, "dds.qos.command");
        expect_error([] { parse_dds_config(YAML::Load("dds: {qos: {command: {durability: invalid}}}")); }, "dds.qos.command");
        expect_error([] { parse_dds_config(YAML::Load("dds: {domain_id: -1}")); }, "dds.domain_id");
        expect_error([] { validate(parse_dds_config(YAML::Load("dds: {domain_id: 233}"))); }, "dds.domain_id");
        std::cout << "DDS configuration tests passed\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
