#pragma once
#include <dds/dds.hpp>

#include <advrf_dds_common/config/dds_config.hpp>

namespace advrf::cyclonedds_plugin {
template<class Qos>
inline Qos apply_endpoint_policy(Qos qos, const advrf::dds_common::EndpointPolicy& policy) {
    if (!policy.is_valid()) throw std::invalid_argument("Invalid DDS history depth");
    qos << dds::core::policy::History::KeepLast(static_cast<int32_t>(policy.history_depth));
    if (policy.reliability == advrf::dds_common::Reliability::Reliable)
        qos << dds::core::policy::Reliability::Reliable();
    else qos << dds::core::policy::Reliability::BestEffort();
    switch (policy.durability) {
        case advrf::dds_common::Durability::Volatile: qos << dds::core::policy::Durability::Volatile(); break;
        case advrf::dds_common::Durability::TransientLocal: qos << dds::core::policy::Durability::TransientLocal(); break;
        case advrf::dds_common::Durability::Transient: qos << dds::core::policy::Durability::Transient(); break;
        case advrf::dds_common::Durability::Persistent: qos << dds::core::policy::Durability::Persistent(); break;
    }
    return qos;
}
} // namespace advrf::cyclonedds_plugin
