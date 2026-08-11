#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
#include <advrf_interfaces/msg/MotorXtTxPdo.hpp>
#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_interfaces/msg/MotorXtTxPdoPubSubTypes.hpp>
#include <advrf_interfaces/msg/MotorTxPdoPubSubTypes.hpp>

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                                             eprosima::fastdds::dds::DomainParticipant* participant) {

        register_subscriber<
            advrf_interfaces::msg::dds_::MotorXtTxPdoVector_,
            advrf_interfaces::msg::dds_::MotorXtTxPdoVector_PubSubType
        >(
            config_topics.command.motorXtCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorXtTxPdoVector_& msg) {
                std::vector<iit::advrf::Ec_slave_pdo> pdos;
                for (const auto& motor_xt_tx_pdo : msg.data()) {
                    iit::advrf::Ec_slave_pdo pdo;
                    convert::protobuf::from_dds(motor_xt_tx_pdo, pdo);
                    pdos.emplace_back(std::move(pdo));
                }
                return pdos;
            });

        register_subscriber<
            advrf_interfaces::msg::dds_::MotorTxPdoVector_,
            advrf_interfaces::msg::dds_::MotorTxPdoVector_PubSubType
        >(
            config_topics.command.motorCmd(), participant, 
            ChannelTx::Motor, [](const advrf_interfaces::msg::dds_::MotorTxPdoVector_& msg) {
                std::vector<iit::advrf::Ec_slave_pdo> pdos;
                for (const auto& motor_tx_pdo : msg.data()) {
                    iit::advrf::Ec_slave_pdo pdo;
                    convert::protobuf::from_dds(motor_tx_pdo, pdo);
                    pdos.emplace_back(std::move(pdo));
                }
                return pdos;
            });
    }

void DDSAdapterSubscribers::spin_once() { 
    for (const auto& sub : subscribers_)
        sub->spin_once(); 
}