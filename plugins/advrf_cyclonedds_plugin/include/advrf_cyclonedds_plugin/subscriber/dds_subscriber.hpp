#pragma once

#include <string>
#include <dds/dds.hpp>
#include <optional>
#include <functional>
#include <utility>

#include <advrf_dds_common/qos/reader_policy.hpp>
#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::cyclonedds_plugin {

/**
 * @brief Common interface for DDS subscribers polled by an adapter.
 */
class DDSSubscriberBase {
public:
    virtual ~DDSSubscriberBase() = default;

    /// Read and process all currently available DDS samples.
    virtual void spin_once() = 0;
};

/**
 * @brief Generic polling DDS subscriber.
 *
 * @tparam Msg DDS message type read from the configured topic.
 *
 * Samples are processed only when @c spin_once is called; callbacks do not
 * run in a separate DDS listener thread.
 */
struct DDSSubscriberOptions
{
    
    std::optional<std::string> user_data;
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

    /**
     * @brief Create the DDS topic, subscriber, and reader.
     *
     * @return True on success; false if DDS entity creation fails.
     */
    bool init_dds(const std::string& topic_name,
                  dds::domain::DomainParticipant& participant,
                  const advrf::dds_common::ReaderPolicy& policy = {},
                  const DDSSubscriberOptions& options = {
                    .user_data = default_user_data_
                  }){
        try
        {
            topic_ = dds::topic::Topic<Msg>(participant, topic_name);
            subscriber_ = dds::sub::Subscriber(participant);

            auto qos = reader_qos(policy);
            if(options.user_data) {
                const auto& value = *options.user_data;
                qos << dds::core::policy::UserData(
                    dds::core::ByteSeq(
                        value.begin(),
                        value.end()
                    )
                );
            }
            reader_ = dds::sub::DataReader<Msg>(subscriber_, topic_, qos);

            return true;
        }
        catch (const dds::core::Exception& e)
        {
            LOG_ERROR("DDS Sub Init Error: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Build reader QoS from a middleware reader policy.
     *
     * Configures history depth and reliable or best-effort delivery by default.
     */
    dds::sub::qos::DataReaderQos reader_qos(const advrf::dds_common::ReaderPolicy& policy) const {
        auto qos = dds::sub::qos::DataReaderQos()
            << dds::core::policy::History::KeepLast(policy.history_depth);

        if (policy.reliability == advrf::dds_common::Reliability::Reliable) {
            qos << dds::core::policy::Reliability::Reliable();
        } else {
            qos << dds::core::policy::Reliability::BestEffort();
        }
        
        return qos;
    }

    /// Set the callback invoked for each valid received sample.
    void set_callback(Callback cb){
        callback_ = std::move(cb);
    }

    /// Take and deliver all valid samples currently available from DDS.
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

    /// Return the underlying DDS reader for graph-bridge registration.
    dds::sub::DataReader<Msg>& dds_reader()
    {
        return reader_;
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
    inline static const std::string default_user_data_ = "advrf=1;";

};

}