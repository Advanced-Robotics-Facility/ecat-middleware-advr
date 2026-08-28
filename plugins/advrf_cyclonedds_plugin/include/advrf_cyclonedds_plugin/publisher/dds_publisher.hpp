#pragma once

#include <string>
#include <optional>
#include <dds/dds.hpp>

#include <advrf_middleware_core/utils/log.hpp>

struct DDSPublisherOptions
{
    std::optional<std::string> user_data;
};

template <typename Msg>
class DDSPublisher {
    public:

        explicit DDSPublisher()
            : publisher_(dds::core::null)
            , topic_(dds::core::null)
            , writer_(dds::core::null)
        {}

        virtual ~DDSPublisher() = default;

        bool init_dds(const std::string& topic_name, 
                      dds::domain::DomainParticipant& participant, 
                      const DDSPublisherOptions& options = {
                        .user_data = default_user_data_
                      })
        {
            try {
                topic_ = dds::topic::Topic<Msg>(
                    participant,
                    topic_name
                );

                publisher_ = dds::pub::Publisher(participant);
                auto qos = writer_qos();
                if (options.user_data) {
                    const auto& value = *options.user_data;
                    qos << dds::core::policy::UserData(
                        dds::core::ByteSeq(
                            value.begin(),
                            value.end()
                        )
                    );
                }

                writer_ = dds::pub::DataWriter<Msg>(
                    publisher_,
                    topic_,
                    qos
                );

                return true;
            }
            catch (const dds::core::Exception& e) {
                LOG_ERROR("DDS Pub Init Error: {}", e.what());
                return false;
            }
        }

        dds::pub::qos::DataWriterQos writer_qos()
        {
            return dds::pub::qos::DataWriterQos()
                << dds::core::policy::Reliability::BestEffort()
                << dds::core::policy::History::KeepLast(1);
        }

        void publish(const Msg& msg) {
            try {
                writer_.write(msg);
            } 
            catch (const dds::core::Exception& e) {
                LOG_ERROR("DDS Pub Write Error: {}", e.what());
            }
        }

        dds::pub::DataWriter<Msg>& dds_writer()
        {
            return writer_;
        }

    protected:

        dds::pub::Publisher publisher_;
        dds::topic::Topic<Msg> topic_;
        dds::pub::DataWriter<Msg> writer_;

    private:
        inline static const std::string default_user_data_ = "advrf=1;";  
};