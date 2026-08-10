#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorXtTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                                             dds::domain::DomainParticipant& participant) {                         
        
        register_subscriber<advrf_interfaces::msg::dds_::MotorXtTxPdo_Vector_>(config_topics.command.motorXtCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorXtTxPdo_Vector_& msg) {
                return msg.data();
            });

        register_subscriber<advrf_interfaces::msg::dds_::MotorTxPdo_Vector_>(config_topics.command.motorCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorTxPdo_Vector_& msg) {
                return msg.data();
            });
    }

void DDSAdapterSubscribers::spin_once() { 
    for (const auto& sub : subscribers_) {
        sub->spin_once();
    }
}