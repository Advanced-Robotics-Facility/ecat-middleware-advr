#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <utility>

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

#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::fastdds_plugin {

template<typename Request, typename RequestPubSubType,
         typename Response, typename ResponsePubSubType>
class ServiceServer
{
public:
    using Callback = std::function<Response(const Request&)>;

    ServiceServer(
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

        subscriber_ = participant_->create_subscriber(
            eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        publisher_ = participant_->create_publisher(
            eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);

        eprosima::fastdds::dds::DataReaderQos rqos =
            eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        rqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        rqos.history().depth = 10;
        reader_ = subscriber_->create_datareader(request_topic_, rqos, nullptr);

        eprosima::fastdds::dds::DataWriterQos wqos =
            eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        wqos.history().depth = 10;
        writer_ = publisher_->create_datawriter(reply_topic_, wqos, nullptr);
    }

    ~ServiceServer() {
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

    void set_callback(Callback callback)
    {
        callback_ = std::move(callback);
    }

    void spin_once()
    {
        if (reader_ == nullptr) return;

        Request request{};
        eprosima::fastdds::dds::SampleInfo info;

        while (reader_->take_next_sample(&request, &info) == eprosima::fastdds::dds::RETCODE_OK)
        {
            if (!info.valid_data || !callback_)
                continue;

            Response response = callback_(request);
            writer_->write(&response);
        }
    }

    void spin(std::chrono::milliseconds period = std::chrono::milliseconds(10))
    {
        while (true)
        {
            spin_once();
            std::this_thread::sleep_for(period);
        }
    }

    eprosima::fastdds::dds::DataReader* dds_reader() { return reader_; }
    eprosima::fastdds::dds::DataWriter* dds_writer() { return writer_; }

private:
    Callback callback_{};

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;

    eprosima::fastdds::dds::Topic* request_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* reply_topic_ = nullptr;

    eprosima::fastdds::dds::DataReader* reader_ = nullptr;
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
};

template<typename Request, typename RequestPubSubType,
         typename Response, typename ResponsePubSubType>
class ServiceServerNotSequential
{
public:
    using Callback = std::function<void(const Request&)>;

    ServiceServerNotSequential  (
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

        subscriber_ = participant_->create_subscriber(
            eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);
        publisher_ = participant_->create_publisher(
            eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);

        eprosima::fastdds::dds::DataReaderQos rqos =
            eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        rqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        rqos.history().depth = 10;
        reader_ = subscriber_->create_datareader(request_topic_, rqos, nullptr);

        eprosima::fastdds::dds::DataWriterQos wqos =
            eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
        wqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        wqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
        wqos.history().depth = 10;
        writer_ = publisher_->create_datawriter(reply_topic_, wqos, nullptr);
    }

    ~ServiceServerNotSequential() {
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

    void set_callback(Callback callback)
    {
        callback_ = std::move(callback);
    }

    std::vector<Request> update_requests(std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        std::vector<Request> requests;
        if (reader_ == nullptr) return requests;

        Request request{};
        eprosima::fastdds::dds::SampleInfo info;

        while (reader_->take_next_sample(&request, &info) ==
               eprosima::fastdds::dds::RETCODE_OK)
        {
            if (info.valid_data)
            {
                requests.push_back(request);
            }
        }
        return requests;
    }

    void process_requests(const std::vector<Request>& requests)
    {
        for (const auto& request : requests)
        {
            if (!callback_)
                continue;

            callback_(request);
        }
    }

    void process_responses(){
        for(const auto& response_pair : response_map_){
            response_pair.second.request_id(response_pair.first);
            publish_response(response_pair.second);
        }
        response_map_.clear();
    }

    void publish_response(const Response& response)
    {
        if (writer_ != nullptr) {
            writer_->write(&response);
        }
    }

    void spin(std::chrono::milliseconds period = std::chrono::milliseconds(10))
    {
        while (true)
        {
            auto requests = update_requests();
            process_requests(requests);
            process_responses();
            std::this_thread::sleep_for(period);
        }
    }

    void add_response(long long request_id, const Response& response)
    {
        response_map_[request_id] = response;
    }

private:
    Callback callback_{};

    std::unordered_map<long long, Response> response_map_;

    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;

    eprosima::fastdds::dds::Topic* request_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* reply_topic_ = nullptr;

    eprosima::fastdds::dds::DataReader* reader_ = nullptr;
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
};

}