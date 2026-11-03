#pragma once
#include <optional>
#include <string>
#include <dds/dds.hpp>

#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/config/qos.hpp"

namespace advrf::cyclonedds_plugin {

struct DDSPublisherOptions
{
    std::optional<std::string> user_data{std::string{"advrf=1;"}};
};

/**
 * @brief Generic DDS writer for a message type.
 *
 * @tparam Msg DDS message type written by this publisher.
 *
 */
template <typename Msg>
class DDSPublisher {
    public:

        explicit DDSPublisher()
            : publisher_(dds::core::null)
            , topic_(dds::core::null)
            , writer_(dds::core::null)
        {}

        virtual ~DDSPublisher() = default;

        /**
         * @brief Create the DDS topic, publisher, and data writer.
         *
         * @param topic_name DDS topic name.
         * @param participant DDS participant that owns the created entities.
         * @param options Optional writer configuration, including user data.
         * @return True on success; false if DDS entity creation throws an exception.
         */
        bool init_dds(const std::string& topic_name, 
                      dds::domain::DomainParticipant& participant, 
                      const DDSPublisherOptions& options = {},
                      const advrf::dds_common::EndpointPolicy& policy = {})
        {
            try {
                topic_ = dds::topic::Topic<Msg>(
                    participant,
                    topic_name
                );

                publisher_ = dds::pub::Publisher(participant);
                auto qos = writer_qos(policy);
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

        /**
         * @brief Return the QoS used by the writer.
         *
         * Uses best-effort reliability and history depth one by default.
         */
        dds::pub::qos::DataWriterQos writer_qos(const advrf::dds_common::EndpointPolicy& policy = {})
        {
            return apply_endpoint_policy(dds::pub::qos::DataWriterQos(), policy);
        }

        /// Publish one DDS message.
        void publish(const Msg& msg) {
            try {
                writer_.write(msg);
            } 
            catch (const dds::core::Exception& e) {
                LOG_ERROR("DDS Pub Write Error: {}", e.what());
            }
        }

        /// Return the underlying DDS data writer.
        dds::pub::DataWriter<Msg>& dds_writer()
        {
            return writer_;
        }

    protected:

        dds::pub::Publisher publisher_;
        dds::topic::Topic<Msg> topic_;
        dds::pub::DataWriter<Msg> writer_;

};

}