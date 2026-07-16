#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <dds/dds.hpp>
#include <dds/dds.h>

template<typename Request, typename Response>
class ServiceClient
{
public:
    ServiceClient(
        dds::domain::DomainParticipant& participant,
        const std::string& request_topic_name,
        const std::string& reply_topic_name)
        : publisher_(participant)
        , subscriber_(participant)
        , request_topic_(participant, request_topic_name)
        , reply_topic_(participant, reply_topic_name)
        , writer_(
              publisher_,
              request_topic_,
              dds::pub::qos::DataWriterQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
        , reader_(
              subscriber_,
              reply_topic_,
              dds::sub::qos::DataReaderQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
    {
        c_reader_ = reader_.delegate().get()->get_ddsc_entity();
        c_writer_ = writer_.delegate().get()->get_ddsc_entity();
    }

    std::optional<Response> call(
        const Request& request,
        std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        dds_write(c_writer_, &request);

        Response response{};
        void* samples[1] = { &response };
        dds_sample_info_t infos[1];

        const auto start = std::chrono::steady_clock::now();

        while (true)
        {
            const int n = dds_take(c_reader_, samples, infos, 1, 1);

            for (int i = 0; i < n; ++i)
            {
                if (infos[i].valid_data)
                    return response;
            }

            if (std::chrono::steady_clock::now() - start >= timeout)
                return std::nullopt;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    dds::pub::Publisher publisher_;
    dds::sub::Subscriber subscriber_;

    dds::topic::Topic<Request> request_topic_;
    dds::topic::Topic<Response> reply_topic_;

    dds::pub::DataWriter<Request> writer_;
    dds::sub::DataReader<Response> reader_;

    dds_entity_t c_reader_{};
    dds_entity_t c_writer_{};
};