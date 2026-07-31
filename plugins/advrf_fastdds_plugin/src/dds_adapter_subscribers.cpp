#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
#include "advrf_dds_common/converter/converter.hpp"

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                                             eprosima::fastdds::dds::DomainParticipant* participant) {
        subscriber_.init_dds(config_topics.command.jointCmd(), participant);                                
        subscriber_.set_callback([this](const MessageDDS& msg) {
            try {
                MessageProtobuf pb_msg;
                convert::protobuf::from_dds(msg, pb_msg);
                this->forward(pb_msg);
            } catch (const std::exception& e) {
                LOG_ERROR("Error processing DDS message: {}", e.what());
            }
            
        });
    }

void DDSAdapterSubscribers::spin_once() { subscriber_.spin_once(); }