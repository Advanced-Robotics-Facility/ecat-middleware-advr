#pragma once

#include <string>
#include <functional>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <advrf_dds_common/qos/reader_policy.hpp>
#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::fastdds_plugin {

class DDSSubscriberBase {
public:
    virtual ~DDSSubscriberBase() = default;

    virtual void spin_once() = 0;
};

template <typename Msg, typename MsgPubSubType>
class DDSSubscriber : public DDSSubscriberBase {
public:

    using Callback = std::function<void(const Msg&)>;

    virtual ~DDSSubscriber() {
        if (reader_ != nullptr && subscriber_ != nullptr) {
            subscriber_->delete_datareader(reader_);
        }
        if (subscriber_ != nullptr && participant_ != nullptr) {
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr && participant_ != nullptr) {
            participant_->delete_topic(topic_);
        }
    }

    bool init_dds(const std::string& topic_name,
                  eprosima::fastdds::dds::DomainParticipant* participant,
                  const advrf::dds_common::ReaderPolicy& policy = {}){
        try
        {
            if (participant == nullptr) {
                LOG_ERROR("DDS Sub Init Error: participant is null");
                return false;
            }
            if (!policy.is_valid()) {
                LOG_ERROR("DDS Sub Init Error: history depth is outside the supported range");
                return false;
            }

            participant_ = participant;

            eprosima::fastdds::dds::TypeSupport type(new MsgPubSubType());
            type.register_type(participant_);

            topic_ = participant_->create_topic(
                topic_name, 
                type.get_type_name(),
                eprosima::fastdds::dds::TOPIC_QOS_DEFAULT
            );
            if (topic_ == nullptr) {
                LOG_ERROR("DDS Sub Init Error: failed to create topic {}", topic_name);
                return false;
            }

            subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
            if (subscriber_ == nullptr) {
                LOG_ERROR("DDS Sub Init Error: failed to create subscriber");
                return false;
            }

            const auto qos = reader_qos(policy);
            reader_ = subscriber_->create_datareader(topic_, qos, nullptr);
            if (reader_ == nullptr) {
                LOG_ERROR("DDS Pub Init Error: failed to create datareader");
                return false;
            }

            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("DDS Sub Init Error: {}", e.what());
            return false;
        }
    }

    eprosima::fastdds::dds::DataReaderQos reader_qos(const advrf::dds_common::ReaderPolicy& policy) const {
        auto qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        qos.reliability().kind =
            policy.reliability == advrf::dds_common::Reliability::Reliable
                ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS
                : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        qos.history().depth = static_cast<int32_t>(policy.history_depth);
        return qos;
    }

    void set_callback(Callback cb){
        callback_ = std::move(cb);
    }

    void spin_once() override
    {
        if (reader_ == nullptr) return;
        Msg sample;
        eprosima::fastdds::dds::SampleInfo info;
        while (reader_->take_next_sample(&sample, &info) == eprosima::fastdds::dds::RETCODE_OK) {
            if (info.valid_data && callback_) {
                callback_(sample);
            }
        }
    }

    eprosima::fastdds::dds::DataReader* dds_reader()
    {
        return reader_;
    }


protected:

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::Topic* topic_ = nullptr;
    eprosima::fastdds::dds::DataReader* reader_ = nullptr;

    bool take(Msg& msg)
    {
        if (reader_ == nullptr) return false;

        eprosima::fastdds::dds::SampleInfo info;
        while (reader_->take_next_sample(&msg, &info) ==
               eprosima::fastdds::dds::RETCODE_OK) {
            if (info.valid_data) {
                return true;
            }
        }
        return false;
    }

private:
    Callback callback_;

};

}