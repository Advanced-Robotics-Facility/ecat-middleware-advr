#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

template<typename Request, typename RequestPubSubType,
         typename Response, typename ResponsePubSubType>
class ServiceClient
{
public:
    ServiceClient(
        eprosima::fastdds::dds::DomainParticipant* participant,
        const std::string& request_topic_name,
        const std::string& reply_topic_name)
        : participant_(participant)
    {
        eprosima::fastdds::dds::TypeSupport req_type(new RequestPubSubType());
        req_type.register_type(participant_);
        eprosima::fastdds::dds::TypeSupport resp_type(new ResponsePubSubType());
        resp_type.register_type(participant_);

        request_topic_ = participant_->create_topic(
            request_topic_name, req_type.get_type_name(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        reply_topic_ = participant_->create_topic(
            reply_topic_name, resp_type.get_type_name(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        publisher_ = participant_->create_publisher(
            eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
        subscriber_ = participant_->create_subscriber(
            eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

        eprosima::fastdds::dds::DataWriterQos wqos =
            eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        wqos.history().depth = 10;
        writer_ = publisher_->create_datawriter(request_topic_, wqos, nullptr);

        eprosima::fastdds::dds::DataReaderQos rqos =
            eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        rqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        rqos.history().depth = 10;
        reader_ = subscriber_->create_datareader(reply_topic_, rqos, nullptr);
    }

    ~ServiceClient() {
        if (reader_ && subscriber_) 
            subscriber_->delete_datareader(reader_);

        if (writer_ && publisher_) 
            publisher_->delete_datawriter(writer_);

        if (subscriber_ && participant_) 
            participant_->delete_subscriber(subscriber_);

        if (publisher_ && participant_) 
            participant_->delete_publisher(publisher_);

        if (request_topic_ && participant_) 
            participant_->delete_topic(request_topic_);

        if (reply_topic_ && participant_) 
            participant_->delete_topic(reply_topic_);
    }

    std::optional<Response> call(
        const Request& request,
        std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        if (writer_ == nullptr || reader_ == nullptr) 
            return std::nullopt;

        writer_->write(&request);

        Response response{};
        eprosima::fastdds::dds::SampleInfo info;

        const auto start = std::chrono::steady_clock::now();

        while (true)
        {
            while (reader_->take_next_sample(&response, &info) == eprosima::fastdds::dds::RETCODE_OK)
            {
                if (info.valid_data)
                    return response;
            }

            if (std::chrono::steady_clock::now() - start >= timeout)
                return std::nullopt;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::Topic* request_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* reply_topic_ = nullptr;

    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
    eprosima::fastdds::dds::DataReader* reader_ = nullptr;
};