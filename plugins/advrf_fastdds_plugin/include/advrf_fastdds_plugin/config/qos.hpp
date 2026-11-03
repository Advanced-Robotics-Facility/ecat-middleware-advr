#pragma once
#include <algorithm>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <advrf_dds_common/config/dds_config.hpp>

namespace advrf::fastdds_plugin {
template<class Qos>
inline Qos apply_endpoint_policy(Qos qos, const advrf::dds_common::EndpointPolicy& policy) {
    using namespace eprosima::fastdds::dds;
    if (!policy.is_valid()) throw std::invalid_argument("Invalid DDS history depth");
    qos.reliability().kind = policy.reliability == advrf::dds_common::Reliability::Reliable
        ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
    switch (policy.durability) {
        case advrf::dds_common::Durability::Volatile: qos.durability().kind = VOLATILE_DURABILITY_QOS; break;
        case advrf::dds_common::Durability::TransientLocal: qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
        case advrf::dds_common::Durability::Transient: qos.durability().kind = TRANSIENT_DURABILITY_QOS; break;
        case advrf::dds_common::Durability::Persistent: qos.durability().kind = PERSISTENT_DURABILITY_QOS; break;
    }
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = static_cast<int32_t>(policy.history_depth);
    // Keep native resource limits compatible with the selected history depth.
    if (qos.resource_limits().max_samples >= 0)
        qos.resource_limits().max_samples = std::max(qos.resource_limits().max_samples, qos.history().depth);
    if (qos.resource_limits().max_samples_per_instance >= 0)
        qos.resource_limits().max_samples_per_instance = std::max(qos.resource_limits().max_samples_per_instance, qos.history().depth);
    if (qos.resource_limits().max_samples >= 0)
        qos.resource_limits().allocated_samples = std::min(qos.resource_limits().allocated_samples, qos.resource_limits().max_samples);
    return qos;
}
} // namespace advrf::fastdds_plugin
