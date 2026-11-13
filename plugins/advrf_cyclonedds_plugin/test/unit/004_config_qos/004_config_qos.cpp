#include <iostream>

#include <advrf_middleware_core/test/config_test_support.hpp>

#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"
#include "advrf_cyclonedds_plugin/config/qos.hpp"

int main() {
    using namespace dds::core::policy;
    try {
        advrf::dds_common::EndpointPolicy policy{advrf::dds_common::Reliability::Reliable,
                                                advrf::dds_common::Durability::TransientLocal, 12};
        const auto writer = advrf::cyclonedds_plugin::apply_endpoint_policy(dds::pub::qos::DataWriterQos(), policy);
        const auto reader = advrf::cyclonedds_plugin::apply_endpoint_policy(dds::sub::qos::DataReaderQos(), policy);
        require(writer.policy<History>().depth() == 12 && reader.policy<History>().depth() == 12, "native history depth");
        require(writer.policy<Reliability>() == Reliability::Reliable() && reader.policy<Reliability>() == Reliability::Reliable(), "native reliability");
        require(writer.policy<Durability>() == Durability::TransientLocal() && reader.policy<Durability>() == Durability::TransientLocal(), "native durability");
        const auto defaults = advrf::cyclonedds_plugin::apply_endpoint_policy(dds::pub::qos::DataWriterQos(), {});
        require(defaults.policy<Reliability>() == Reliability::BestEffort() && defaults.policy<History>().depth() == 1, "native defaults");
        std::cout << "CycloneDDS QoS tests passed\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
