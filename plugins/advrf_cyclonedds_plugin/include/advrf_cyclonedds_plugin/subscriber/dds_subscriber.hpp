#pragma once

#include <string>
#include <dds/dds.hpp>

#include <advrf_middleware_core/utils/log.hpp>

template <typename Msg>
class DDSSubscriber {
public:

    using Callback = std::function<void(const Msg&)>;

    DDSSubscriber()
        : subscriber_(dds::core::null)
        , topic_(dds::core::null)
        , reader_(dds::core::null)
    {}

    virtual ~DDSSubscriber() = default;

    bool init_dds(const std::string& topic_name,
                  dds::domain::DomainParticipant& participant){
        try
        {
            topic_ = dds::topic::Topic<Msg>(participant, topic_name);
            subscriber_ = dds::sub::Subscriber(participant);
            reader_ = dds::sub::DataReader<Msg>(subscriber_, topic_, reader_qos());

            return true;
        }
        catch (const dds::core::Exception& e)
        {
            LOG_ERROR("DDS Sub Init Error: {}", e.what());
            return false;
        }
    }

    dds::sub::qos::DataReaderQos reader_qos(){
        return dds::sub::qos::DataReaderQos()
            << dds::core::policy::Reliability::BestEffort()
            << dds::core::policy::History::KeepLast(1);
    }

    void set_callback(Callback cb){
        callback_ = std::move(cb);
    }

    void spin_once()
    {
        auto samples = reader_.take();
        for (const auto& sample : samples)
        {
            if (sample.info().valid() && callback_)
            {
                callback_(sample.data());
            }
        }
    }


protected:

    dds::sub::Subscriber subscriber_;
    dds::topic::Topic<Msg> topic_;
    dds::sub::DataReader<Msg> reader_;

    bool take(Msg& msg)
    {
        auto samples = reader_.take();
        for (const auto& sample : samples)
        {
            if (sample.info().valid())
            {
                msg = sample.data();
                return true;
            }
        }
        return false;
    }

private:
    Callback callback_;

};