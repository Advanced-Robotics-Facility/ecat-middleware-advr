#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <utility>
#include <optional>

#include <dds/dds.hpp>
#include <dds/dds.h>

template<typename Request, typename Response>
class ServiceServer
{
public:
    using Callback = std::function<Response(const Request&)>;

    ServiceServer(
        dds::domain::DomainParticipant& participant,
        const std::string& request_topic,
        const std::string& reply_topic)
        : reader_(
              dds::sub::Subscriber(participant),
              dds::topic::Topic<Request>(participant, request_topic),
              dds::sub::qos::DataReaderQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
        , writer_(
              dds::pub::Publisher(participant),
              dds::topic::Topic<Response>(participant, reply_topic),
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

    void spin_once()
    {
        void* samples[] = { &request_ };
        dds_sample_info_t infos[1];

        int n = dds_take(c_reader_, samples, infos, 1, 1);

        for (int i = 0; i < n; ++i)
        {
            if (!infos[i].valid_data)
                continue;

            if (!callback_)
                continue;

            Response response = callback_(request_);

            dds_write(c_writer_, &response);
        }
    }

    void spin(std::chrono::milliseconds period = std::chrono::milliseconds(1))
    {
        while (true)
        {
            spin_once();
            std::this_thread::sleep_for(period);
        }
    }

private:
    Callback callback_;

    dds::sub::DataReader<Request> reader_;
    dds::pub::DataWriter<Response> writer_;
    dds_entity_t c_reader_{};
    dds_entity_t c_writer_{};
    Request request_{};
};


template<typename Request, typename Response>
class ServiceClient
{
public:
    ServiceClient(
        dds::domain::DomainParticipant& participant,
        const std::string& request_topic,
        const std::string& reply_topic)
        : writer_(
              dds::pub::Publisher(participant),
              dds::topic::Topic<Request>(participant, request_topic),
              dds::pub::qos::DataWriterQos()
                  << dds::core::policy::Reliability::Reliable()
                  << dds::core::policy::History::KeepLast(10))
        , reader_(
              dds::sub::Subscriber(participant),
              dds::topic::Topic<Response>(participant, reply_topic),
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

    void* samples[] = { &response_ };
    dds_sample_info_t infos[1];

    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        int n = dds_take(c_reader_, samples, infos, 1, 1);
        if (n > 0 && infos[0].valid_data)
            return response_;

        if (std::chrono::steady_clock::now() - start >= timeout)
            return std::nullopt;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    }



private:
    dds::pub::DataWriter<Request> writer_;
    dds::sub::DataReader<Response> reader_;

    dds_entity_t c_reader_{};
    dds_entity_t c_writer_{};

    Response response_{};
};