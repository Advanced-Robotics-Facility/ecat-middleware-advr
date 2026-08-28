#pragma once

#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <advrf_middleware_core/utils/log.hpp>

template <typename Msg, typename MsgPubSubType>
class DDSPublisher {
public:

    explicit DDSPublisher() = default;

    virtual ~DDSPublisher() {
        if (writer_ != nullptr && publisher_ != nullptr) {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr && participant_ != nullptr) {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr && participant_ != nullptr) {
            participant_->delete_topic(topic_);
        }
    }

    bool init_dds(const std::string& topic_name,
                    eprosima::fastdds::dds::DomainParticipant* participant) {
        try {
            participant_ = participant;

            eprosima::fastdds::dds::TypeSupport type(new MsgPubSubType());
            type.register_type(participant_);

            topic_ = participant_->create_topic(
                topic_name,
                type.get_type_name(), 
                eprosima::fastdds::dds::TOPIC_QOS_DEFAULT
            );
            if (topic_ == nullptr) {
                LOG_ERROR("DDS Pub Init Error: failed to create topic {}", topic_name);
                return false;
            }

            //LOG_INFO("Topic '{}' created with type '{}'", topic_name, topic_->get_type_name());

            publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
            if (publisher_ == nullptr) {
                LOG_ERROR("DDS Pub Init Error: failed to create publisher");
                return false;
            }

            eprosima::fastdds::dds::DataWriterQos qos = writer_qos();
            writer_ = publisher_->create_datawriter(topic_, qos, nullptr);
            if (writer_ == nullptr) {
                LOG_ERROR("DDS Pub Init Error: failed to create datawriter");
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("DDS Pub Init Error: {}", e.what());
            return false;
        }
    }

    eprosima::fastdds::dds::DataWriterQos writer_qos() {
        auto qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
        qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        qos.history().depth = 1;
        qos.resource_limits().max_samples = 1;
        qos.resource_limits().allocated_samples = 1;
        return qos;
    }

    void publish(const Msg& msg) {
        if (writer_ == nullptr) {
            LOG_ERROR("DDS Pub Write Error: writer not initialized");
            return;
        }
        eprosima::fastdds::dds::ReturnCode_t ret = writer_->write(&msg);
        if (ret != eprosima::fastdds::dds::RETCODE_OK) {
            LOG_ERROR("DDS Pub Write Error: write() retcode {}", ret);
        }
    }

    eprosima::fastdds::dds::DataWriter* dds_writer()
    {
        return writer_;
    }
  

protected:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Topic* topic_ = nullptr;
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
};