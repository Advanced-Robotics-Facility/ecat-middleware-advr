#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorTxPdo.hpp>

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                                             dds::domain::DomainParticipant& participant,
                                             advrf::dds_common::ReaderPolicy reader_policy)
    : reader_policy_(reader_policy) {
        
        register_subscriber<advrf_interfaces::msg::dds_::MotorTxPdoVector_>(
            config_topics.command.motorXtCmd(),
            participant,
            ChannelTx::Motor,
            [](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
                return convert::protobuf::vector_from_dds(
                    msg, iit::advrf::Ec_slave_pdo::TX_XT_MOTOR);
            });

        register_subscriber<advrf_interfaces::msg::dds_::MotorTxPdoVector_>(
            config_topics.command.motorCmd(),
            participant,
            ChannelTx::Motor,
            [](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
                return convert::protobuf::vector_from_dds(msg);
            });
    }

void DDSAdapterSubscribers::spin_once() { 
    for (const auto& sub : subscribers_) {
        sub->spin_once();
    }
}
