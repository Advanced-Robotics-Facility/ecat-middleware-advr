#pragma once

#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <dds/dds.h>
#include <dds/dds.hpp>

#include <rmw_dds_common/msg/ParticipantEntitiesInfo.hpp>
#include <advrf_dds_common/ros_metadata/ros_graph_state.hpp>

class CycloneDDSRosGraphBridge : private RosGraphState {
public:
    using GidMsg = rmw_dds_common::msg::dds_::Gid_;
    using NodeInfo = rmw_dds_common::msg::dds_::NodeEntitiesInfo_;
    using ParticipantInfo = rmw_dds_common::msg::dds_::ParticipantEntitiesInfo_;

    static std::string build_node_namespace(
        const std::string& ns,
        const std::string& robot_name)
    {
        return "/" + ns + "/cyclonedds/" + robot_name;
    }

    CycloneDDSRosGraphBridge(
        dds::domain::DomainParticipant& participant,
        std::string node_name,
        std::string node_namespace)
        : participant_(participant),
          publisher_(participant),
          topic_(participant, "ros_discovery_info"),
          writer_(publisher_, topic_, discovery_qos()),
          node_name_(std::move(node_name)),
          node_namespace_(std::move(node_namespace)),
          participant_gid_(get_guid(participant))
    {
        publish();
    }

    template <typename Msg>
    void add_writer(dds::pub::DataWriter<Msg>& writer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        add_writer_gid(get_guid(writer));
        publish_locked();
    }

    template <typename Msg>
    void remove_writer(dds::pub::DataWriter<Msg>& writer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        remove_writer_gid(get_guid(writer));
        publish_locked();
    }

    template <typename Msg>
    void add_reader(dds::sub::DataReader<Msg>& reader)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        add_reader_gid(get_guid(reader));
        publish_locked();
    }

    template <typename Msg>
    void remove_reader(dds::sub::DataReader<Msg>& reader)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        remove_reader_gid(get_guid(reader));
        publish_locked();
    }

    void publish()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        publish_locked();
    }

private:
    static dds::pub::qos::DataWriterQos discovery_qos()
    {
        return dds::pub::qos::DataWriterQos()
            << dds::core::policy::Reliability::Reliable()
            << dds::core::policy::Durability::TransientLocal()
            << dds::core::policy::History::KeepLast(1);
    }

    template <typename Entity>
    static GidBytes get_guid(Entity& entity)
    {
        dds_guid_t guid{};
        const dds_entity_t native_entity = entity.delegate()->get_ddsc_entity();
        const dds_return_t result = dds_get_guid(native_entity, &guid);

        if (result != DDS_RETCODE_OK) {
            throw std::runtime_error(
                "Failed to retrieve CycloneDDS entity GUID"
            );
        }

        GidBytes gid{};
        std::copy_n(guid.v, gid.size(), gid.begin());

        return gid;
    }

    static GidMsg make_gid(const GidBytes& gid)
    {
        return GidMsg(gid);
    }

    void publish_locked()
    {
        std::vector<GidMsg> readers;
        readers.reserve(reader_gids().size());

        for (const auto& gid : reader_gids()) {
            readers.emplace_back(make_gid(gid));
        }

        std::vector<GidMsg> writers;
        writers.reserve(writer_gids().size());

        for (const auto& gid : writer_gids()) {
            writers.emplace_back(make_gid(gid));
        }

        NodeInfo node_info(
            node_namespace_,
            node_name_,
            std::move(readers),
            std::move(writers)
        );

        std::vector<NodeInfo> nodes;
        nodes.emplace_back(std::move(node_info));

        ParticipantInfo participant_info(
            make_gid(participant_gid_),
            std::move(nodes)
        );

        writer_.write(participant_info);
    }

    dds::domain::DomainParticipant& participant_;

    dds::pub::Publisher publisher_;
    dds::topic::Topic<ParticipantInfo> topic_;
    dds::pub::DataWriter<ParticipantInfo> writer_;

    std::string node_name_;
    std::string node_namespace_;

    GidBytes participant_gid_;
    std::mutex mutex_;
};


class IConnectRosGraphBridge {
public:
    virtual ~IConnectRosGraphBridge() = default;
    virtual void connect_ros_graph_bridge(
        CycloneDDSRosGraphBridge& bridge) = 0;
};