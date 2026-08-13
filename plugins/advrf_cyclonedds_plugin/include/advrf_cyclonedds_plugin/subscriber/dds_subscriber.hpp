#pragma once

#include <string>
#include <dds/dds.hpp>

#include <advrf_dds_common/qos/reader_policy.hpp>
#include <advrf_middleware_core/utils/log.hpp>

class DDSSubscriberBase {
public:
    virtual ~DDSSubscriberBase() = default;

    virtual void spin_once() = 0;
};


template <typename Msg>
class DDSSubscriber : public DDSSubscriberBase {
public:

    using Callback = std::function<void(const Msg&)>;

    DDSSubscriber()
        : subscriber_(dds::core::null)
        , topic_(dds::core::null)
        , reader_(dds::core::null)
    {}

    virtual ~DDSSubscriber() = default;

    bool init_dds(const std::string& topic_name,
                  dds::domain::DomainParticipant& participant,
                  const advrf::dds_common::ReaderPolicy& policy = {}){
        try
        {
            topic_ = dds::topic::Topic<Msg>(participant, topic_name);
            subscriber_ = dds::sub::Subscriber(participant);
            reader_ = dds::sub::DataReader<Msg>(subscriber_, topic_, reader_qos(policy));

            return true;
        }
        catch (const dds::core::Exception& e)
        {
            LOG_ERROR("DDS Sub Init Error: {}", e.what());
            return false;
        }
    }

    dds::sub::qos::DataReaderQos reader_qos(const advrf::dds_common::ReaderPolicy& policy) const {
        auto qos = dds::sub::qos::DataReaderQos()
            << dds::core::policy::History::KeepLast(policy.history_depth);

        dds::core::ByteSeq user_data = {
                'a','d','v','r','f','=','1',';'
            };

        if (policy.reliability == advrf::dds_common::Reliability::Reliable) {
            qos << dds::core::policy::Reliability::Reliable();
        } else {
            qos << dds::core::policy::Reliability::BestEffort();
        }

        qos << dds::core::policy::UserData(user_data);
        return qos;
    }

    void set_callback(Callback cb){
        callback_ = std::move(cb);
    }

    void spin_once() override
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