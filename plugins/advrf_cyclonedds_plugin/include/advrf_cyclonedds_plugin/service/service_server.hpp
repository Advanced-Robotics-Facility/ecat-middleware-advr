#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <optional>

#include <dds/dds.hpp>
#include <dds/dds.h>

namespace advrf::cyclonedds_plugin {

/**
 * @brief Polling DDS request/reply service server.
 *
 * @tparam Request DDS request type.
 * @tparam Response DDS response type.
 *
 * Each valid received request is passed to the callback and its returned
 * response is published immediately.
 */
template<typename Request, typename Response>
class ServiceServer
{
public:
    using Callback = std::function<Response(const Request&)>;

    ServiceServer(
        dds::domain::DomainParticipant& participant,
        const std::string& request_topic_name,
        const std::string& reply_topic_name)
        : subscriber_(participant)
        , publisher_(participant)
        , request_topic_(participant, request_topic_name)
        , reply_topic_(participant, reply_topic_name)
        , reader_(
              subscriber_,
              request_topic_,
              dds::sub::qos::DataReaderQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10)
                  << dds::core::policy::UserData(
                      dds::core::ByteSeq(
                          default_user_data_.begin(),
                          default_user_data_.end()
                      )
                  ))
        , writer_(
              publisher_,
              reply_topic_,
              dds::pub::qos::DataWriterQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10)
                  << dds::core::policy::UserData(
                      dds::core::ByteSeq(
                          default_user_data_.begin(),
                          default_user_data_.end()
                      )
                  ))
    {
        c_reader_ = reader_.delegate().get()->get_ddsc_entity();
        c_writer_ = writer_.delegate().get()->get_ddsc_entity();
    }

    /// Set the handler invoked for each valid request.
    void set_callback(Callback callback)
    {
        callback_ = std::move(callback);
    }

    /// Process currently available requests.
    void spin_once()
    {
        Request request{};
        void* samples[1] = { &request };
        dds_sample_info_t infos[1];

        const int n = dds_take(c_reader_, samples, infos, 1, 1);

        for (int i = 0; i < n; ++i)
        {
            if (!infos[i].valid_data)
                continue;

            if (!callback_)
                continue;

            Response response = callback_(request);
            dds_write(c_writer_, &response);
        }
    }

    /// Poll requests continuously.
    void spin(std::chrono::milliseconds period = std::chrono::milliseconds(10))
    {
        while (true)
        {
            spin_once();
            std::this_thread::sleep_for(period);
        }
    }

    dds::sub::DataReader<Request>& dds_reader() { return reader_; }
    dds::pub::DataWriter<Response>& dds_writer() { return writer_; }

private:
    Callback callback_{};

    dds::sub::Subscriber subscriber_;
    dds::pub::Publisher publisher_;

    dds::topic::Topic<Request> request_topic_;
    dds::topic::Topic<Response> reply_topic_;

    dds::sub::DataReader<Request> reader_;
    dds::pub::DataWriter<Response> writer_;

    dds_entity_t c_reader_{};
    dds_entity_t c_writer_{};

    inline static const std::string default_user_data_ = "advrf=1;";
};

/**
 * @brief DDS service helper that separates request handling from responses.
 *
 * Requests are collected and processed first. The callback can later add
 * replies with @ref add_response, which are published by
 * @ref process_responses.
 */
template<typename Request, typename Response>
class ServiceServerNotSequential
{
public:
    using Callback = std::function<void(const Request&)>;

    ServiceServerNotSequential  (
        dds::domain::DomainParticipant& participant,
        const std::string& request_topic_name,
        const std::string& reply_topic_name)
        : subscriber_(participant)
        , publisher_(participant)
        , request_topic_(participant, request_topic_name)
        , reply_topic_(participant, reply_topic_name)
        , reader_(
              subscriber_,
              request_topic_,
              dds::sub::qos::DataReaderQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
        , writer_(
              publisher_,
              reply_topic_,
              dds::pub::qos::DataWriterQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
    {
        c_reader_ = reader_.delegate().get()->get_ddsc_entity();
        c_writer_ = writer_.delegate().get()->get_ddsc_entity();
    }

    void set_callback(Callback callback)
    {
        callback_ = std::move(callback);
    }

    /// Retrieve currently available valid requests.
    std::vector<Request> update_requests(std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        std::vector<Request> requests;
        Request request{};
        void* samples[1] = { &request };
        dds_sample_info_t infos[1];
        const auto start = std::chrono::steady_clock::now();
        const int n = dds_take(c_reader_, samples, infos, 1, 1);
        if(n > 0){
            requests.reserve(n);
            for (int i = 0; i < n; ++i)
            {
                if (infos[i].valid_data)
                {
                    requests.emplace_back(request);
                }
            }
        }
        return requests;
    }

    /// Invoke the configured callback for every request.
    void process_requests(const std::vector<Request>& requests)
    {
        for (const auto& request : requests)
        {
            if (!callback_)
                continue;

            callback_(request);
        }
    }

    /// Publish queued replies and clear the reply queue.
    void process_responses(){
        for(const auto& response_pair : response_map_){
            response_pair.second.request_id(response_pair.first);
            publish_response(response_pair.second);
        }
        response_map_.clear();
    }

    void publish_response(const Response& response)
    {
        dds_write(c_writer_, &response);
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

    /// Queue a reply for the specified request ID.
    void add_response(long long request_id, const Response& response)
    {
        response_map_[request_id] = response;
    }

private:
    Callback callback_{};

    std::unordered_map<long long, Response> response_map_;

    dds::sub::Subscriber subscriber_;
    dds::pub::Publisher publisher_;

    dds::topic::Topic<Request> request_topic_;
    dds::topic::Topic<Response> reply_topic_;

    dds::sub::DataReader<Request> reader_;
    dds::pub::DataWriter<Response> writer_;

    dds_entity_t c_reader_{};
    dds_entity_t c_writer_{};
  
};

}