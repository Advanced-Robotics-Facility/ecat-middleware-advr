#pragma once

#include <string>
#include <dds/dds.hpp>

#include <advrf_middleware_core/utils/log.hpp>

template <typename Msg, typename Derived>
class DDSPublisher {
    public:

        explicit DDSPublisher()
            : publisher_(dds::core::null)
            , topic_(dds::core::null)
            , writer_(dds::core::null)
        {}

        virtual ~DDSPublisher() = default;

        bool init_dds(const std::string& topic_name,
                      dds::domain::DomainParticipant& participant) {
            try {
                topic_ = dds::topic::Topic<Msg>(participant, topic_name);
                publisher_ = dds::pub::Publisher(participant);
                dds::pub::qos::DataWriterQos qos = static_cast<Derived*>(this)->writer_qos();
                writer_ = dds::pub::DataWriter<Msg>(publisher_, topic_, qos);
                return true;
            } 
            catch (const dds::core::Exception& e) {
                LOG_ERROR("DDS Pub Init Error: {}", e.what());
                return false;
            }
        }

        dds::pub::qos::DataWriterQos writer_qos() {
            return dds::pub::qos::DataWriterQos()
                << dds::core::policy::Reliability::BestEffort()
                << dds::core::policy::History::KeepLast(1);
        }

    protected:

        dds::pub::Publisher publisher_;
        dds::topic::Topic<Msg> topic_;
        dds::pub::DataWriter<Msg> writer_;
};