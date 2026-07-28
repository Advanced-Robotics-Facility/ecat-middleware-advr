#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <thread>

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

struct InspectorOptions
{
    bool verbose{true};
    bool json{false};
    bool stats{false};
    bool once{false};
    bool history{false};

    int rate{10};

    std::vector<std::string> filter;
};

template<typename Bridge>
class BridgeInspector
{
public:

    using MessagePtr  = std::unique_ptr<google::protobuf::Message>;
    using MessageList = std::vector<MessagePtr>;
    using MessageMap  = std::unordered_map<std::string, MessageList>;

    explicit BridgeInspector(const std::string& shm_name);

    virtual ~BridgeInspector() = default;

    void run(const InspectorOptions& options);

protected:

    virtual void declare() = 0;

    template<typename Proto, typename Queue>
    void register_queue(const std::string& name, Queue& queue);

    Bridge* bridge_{nullptr};

    ShmProtoHelper proto_helper_;

private:

    struct QueueDescriptor
    {
        std::string name;
        std::string proto_name;

        std::function<void(MessageMap&, const InspectorOptions&)> reader;
    };

    struct QueueStats
    {
        std::size_t total_messages{0};

        std::chrono::steady_clock::time_point first_seen{};
        std::chrono::steady_clock::time_point last_seen{};

        bool initialized{false};
    };

    std::vector<QueueDescriptor> queues_;
    std::unordered_map<std::string, QueueStats> stats_;

    SharedMemoryOpenOrCreate shm_;
    std::string shm_name_;

    void connect();

    MessageMap read(const InspectorOptions& options);

    void update_stats(const MessageMap& messages);

    bool should_display(const std::string& queue,
                        const InspectorOptions& options) const;

    static void clear_screen();

    void print_header(const std::string& title) const;

    void display(const MessageMap& messages,
                 const InspectorOptions& options) const;

    void display_json(const MessageMap& messages,
                  const InspectorOptions& options) const;

    void display_stats(const InspectorOptions& options) const;
};


template<typename Bridge>
BridgeInspector<Bridge>::BridgeInspector(const std::string& shm_name)
    : shm_name_(shm_name),
      shm_(shm_name.c_str(), sizeof(Bridge))
{
}

template<typename Bridge>
void BridgeInspector<Bridge>::connect()
{
    if (!shm_.is_valid())
    {
        throw std::runtime_error(
            "Unable to open shared memory '" + shm_name_ + "'");
    }

    bridge_ = static_cast<Bridge*>(shm_.raw_ptr());
}

template<typename Bridge>
template<typename Proto, typename Queue>
void BridgeInspector<Bridge>::register_queue(const std::string& name, Queue& queue)
{
    queues_.push_back(
    {
        name,
        Proto::descriptor()->full_name(),

        [this, name, &queue]
        (MessageMap& messages,
         const InspectorOptions& options)
        {
            if (options.history)
            {
                Proto proto;

                proto_helper_.peek_all(
                    queue,
                    proto,
                    [&](const Proto& p)
                    {
                        messages[name].push_back(
                            std::make_unique<Proto>(p));
                    });

                return;
            }

            auto msg = std::make_unique<Proto>();

            if (proto_helper_.peek_latest(queue, *msg))
            {
                messages[name].push_back(std::move(msg));
            }
        }
    });
}

template<typename Bridge>
typename BridgeInspector<Bridge>::MessageMap
BridgeInspector<Bridge>::read(const InspectorOptions& options)
{
    MessageMap messages;

    for (const auto& queue : queues_)
        queue.reader(messages, options);

    return messages;
}

template<typename Bridge>
void BridgeInspector<Bridge>::update_stats(
    const MessageMap& messages)
{
    const auto now = std::chrono::steady_clock::now();

    for (const auto& [name, list] : messages)
    {
        auto& stats = stats_[name];

        if (!stats.initialized)
        {
            stats.initialized = true;
            stats.first_seen = now;
        }

        stats.last_seen = now;
        stats.total_messages += list.size();
    }
}

template<typename Bridge>
bool BridgeInspector<Bridge>::should_display(
    const std::string& queue,
    const InspectorOptions& options) const
{
    return options.filter.empty() ||
           std::find(options.filter.begin(),
                     options.filter.end(),
                     queue) != options.filter.end();
}

template<typename Bridge>
void BridgeInspector<Bridge>::clear_screen()
{
    std::cout << "\033[2J\033[H";
}

template<typename Bridge>
void BridgeInspector<Bridge>::print_header(
    const std::string& title) const
{
    clear_screen();

    std::cout
        << "============================================================\n"
        << " " << title << '\n'
        << "============================================================\n";

    std::cout
        << "Shared Memory      : "
        << shm_name_
        << '\n';

    std::cout
        << "Registered Queues  : "
        << queues_.size()
        << "\n\n";
}

template<typename Bridge>
void BridgeInspector<Bridge>::display(
    const MessageMap& messages,
    const InspectorOptions& options) const
{
    print_header("Bridge Inspector");
    std::size_t total = 0;
    for (const auto& queue : queues_)
    {
        if (!should_display(queue.name, options))
            continue;

        const auto it = messages.find(queue.name);

        const MessageList* list = nullptr;
        std::size_t count = 0;

        if (it != messages.end())
        {
            list = &it->second;
            count = list->size();
        }

        total += count;

        std::cout
            << std::left
            << std::setw(20) << queue.name
            << std::setw(45) << queue.proto_name
            << count << " msg\n";

        if (!options.verbose || list == nullptr)
            continue;

        std::size_t index = 0;

        for (const auto& msg : *list)
        {
            if (options.history)
            {
                std::cout
                    << "---------------- Message "
                    << index++
                    << " ----------------\n";
            }
            else
            {
                std::cout
                    << "------------------------------------------------------------\n";
            }

            msg->PrintDebugString();
        }

        std::cout << '\n';
    }

    std::cout
        << "============================================================\n"
        << "Total messages : "
        << total
        << '\n';
}

template<typename Bridge>
void BridgeInspector<Bridge>::display_json(
    const MessageMap& messages,
    const InspectorOptions& options) const
{
    for (const auto& [name, list] : messages)
    {
        std::cout << name << '\n';
        for (const auto& msg : list)
        {
            std::string json;
            google::protobuf::util::MessageToJsonString(
                *msg,
                &json);
            std::cout << json << "\n\n";
        }
    }
}

template<typename Bridge>
void BridgeInspector<Bridge>::display_stats(
    const InspectorOptions& options) const
{
    using namespace std::chrono;
    print_header("Bridge Statistics");
    std::cout
        << std::left
        << std::setw(20) << "Queue"
        << std::setw(18) << "Messages"
        << std::setw(12) << "Msg/s"
        << "Last seen\n";

    std::cout
        << "------------------------------------------------------------\n";

    const auto now = steady_clock::now();

    for (const auto& queue : queues_)
    {
        if (!should_display(queue.name, options))
            continue;

        const auto it = stats_.find(queue.name);

        if (it == stats_.end())
        {
            std::cout
                << std::setw(20) << queue.name
                << std::setw(18) << 0
                << std::setw(12) << 0.0
                << "never\n";

            continue;
        }

        const auto& stats = it->second;

        const double elapsed =
            duration<double>(
                stats.last_seen - stats.first_seen).count();

        const double rate =
            elapsed > 0.0
                ? static_cast<double>(stats.total_messages) / elapsed
                : 0.0;

        const auto age =
            duration_cast<milliseconds>(
                now - stats.last_seen).count();

        std::cout
            << std::setw(20) << queue.name
            << std::setw(18) << stats.total_messages
            << std::setw(12) << std::fixed << std::setprecision(1) << rate
            << age << " ms\n";
    }
}

template<typename Bridge>
void BridgeInspector<Bridge>::run(
    const InspectorOptions& options)
{
    connect();
    declare();

    const auto period =
        std::chrono::milliseconds(
            1000 / std::max(options.rate, 1));

    do
    {
        auto messages = read(options);

        update_stats(messages);

        if (options.stats)
        {
            display_stats(options);
        }
        else if (options.json)
        {
            display_json(messages, options);
        }
        else
        {
            display(messages, options);
        }

        if (!options.once)
            std::this_thread::sleep_for(period);

    } while (!options.once);
}