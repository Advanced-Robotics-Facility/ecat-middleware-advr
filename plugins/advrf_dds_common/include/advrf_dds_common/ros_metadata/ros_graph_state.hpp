#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace advrf::dds_common::ros_metadata {

class RosGraphState {
public:
    using GidBytes = std::array<uint8_t, 16>;

protected:
    void add_reader_gid(const GidBytes& gid)
    {
        add_unique(reader_gids_, gid);
    }

    void add_writer_gid(const GidBytes& gid)
    {
        add_unique(writer_gids_, gid);
    }

    void remove_reader_gid(const GidBytes& gid)
    {
        remove(reader_gids_, gid);
    }

    void remove_writer_gid(const GidBytes& gid)
    {
        remove(writer_gids_, gid);
    }

    const std::vector<GidBytes>& reader_gids() const noexcept
    {
        return reader_gids_;
    }

    const std::vector<GidBytes>& writer_gids() const noexcept
    {
        return writer_gids_;
    }

private:
    static void add_unique(
        std::vector<GidBytes>& container,
        const GidBytes& gid)
    {
        if (std::find(container.begin(), container.end(), gid)
            == container.end())
        {
            container.push_back(gid);
        }
    }

    static void remove(
        std::vector<GidBytes>& container,
        const GidBytes& gid)
    {
        container.erase(
            std::remove(container.begin(), container.end(), gid),
            container.end()
        );
    }

    std::vector<GidBytes> reader_gids_;
    std::vector<GidBytes> writer_gids_;
};

}