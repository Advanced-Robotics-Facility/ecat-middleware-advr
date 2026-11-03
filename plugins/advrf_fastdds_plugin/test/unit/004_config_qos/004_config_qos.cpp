#include <iostream>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <advrf_middleware_core/test/config_test_support.hpp>

#include "advrf_fastdds_plugin/config/plugin_config.hpp"
#include "advrf_fastdds_plugin/config/qos.hpp"

int main() {
    using namespace eprosima::fastdds::dds;
    try {
        advrf::dds_common::EndpointPolicy policy{advrf::dds_common::Reliability::Reliable,
                                                advrf::dds_common::Durability::TransientLocal, 12};
        const auto writer = advrf::fastdds_plugin::apply_endpoint_policy(DATAWRITER_QOS_DEFAULT, policy);
        const auto reader = advrf::fastdds_plugin::apply_endpoint_policy(DATAREADER_QOS_DEFAULT, policy);
        require(writer.history().depth == 12 && reader.history().depth == 12, "native history depth");
        require(writer.reliability().kind == RELIABLE_RELIABILITY_QOS && reader.reliability().kind == RELIABLE_RELIABILITY_QOS, "native reliability");
        require(writer.durability().kind == TRANSIENT_LOCAL_DURABILITY_QOS && reader.durability().kind == TRANSIENT_LOCAL_DURABILITY_QOS, "native durability");
        const auto defaults = advrf::fastdds_plugin::apply_endpoint_policy(DATAWRITER_QOS_DEFAULT, {});
        require(defaults.reliability().kind == BEST_EFFORT_RELIABILITY_QOS && defaults.history().depth == 1, "native defaults");
        std::cout << "FastDDS QoS tests passed\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
