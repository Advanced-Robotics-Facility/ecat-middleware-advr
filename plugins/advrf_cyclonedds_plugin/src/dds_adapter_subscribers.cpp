#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include "advrf_cyclonedds_plugin/converter.hpp"

DDSAdapterSubscribers::DDSAdapterSubscribers(const config::ConfigTopics& config_topics, 
                                             dds::domain::DomainParticipant& participant) {
        subscriber_.init_dds(config_topics.command.jointCmd(), participant);                                
        subscriber_.set_callback([this](const MessageDDS& msg) {
            try {
                MessageProtobuf pb_msg;
                convert::protobuf::from_dds(msg, pb_msg);
                this->forward_ctrl_cmd(pb_msg);
            } catch (const std::exception& e) {
                LOG_ERROR("Error processing DDS message: {}", e.what());
            }
            
        });
    }

void DDSAdapterSubscribers::spin_once() { subscriber_.spin_once(); }