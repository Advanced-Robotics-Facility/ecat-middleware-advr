#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct InspectorOptions
{
    int rate{10};
    bool history{false};
};

struct QueueSnapshot
{
    std::string name;
    std::string proto_name;

    std::size_t buffered{0};
    std::size_t capacity{0};
    std::size_t observed_messages{0};
    double observed_rate{0.0};
    long long age_ms{-1};
    std::vector<std::string> messages;
};

struct InspectorSnapshot
{
    std::string shm_name;
    std::vector<QueueSnapshot> queues;
    std::size_t total_buffered{0};
    bool history{false};
    std::string error;
};