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
#include <yaml-cpp/yaml.h>

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <shm_tools/monitor.hpp>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

struct InspectorOptions
{
    bool verbose{true};
    bool json{false};
    bool stats{false};
    bool once{false};
    bool history{false};
    bool yaml{false};
    bool progress{false};

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
    mutable Console console_;

    struct QueueDescriptor
    {
        std::string name;
        std::string proto_name;
        std::size_t capacity;

        std::function<void(MessageMap&, const InspectorOptions&)> reader;
        std::function<std::size_t()> size;
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

    void display_yaml(const MessageMap& messages,
                    const InspectorOptions& options) const;

    void display_progress(const MessageMap& messages,
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
        Queue::capacity,    

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
        },
         [&queue]()
        {
            return queue.size();
        }
    });
}

template<typename Bridge>
typename BridgeInspector<Bridge>::MessageMap
BridgeInspector<Bridge>::read(const InspectorOptions& options)
{
    MessageMap messages;

    for (const auto& queue : queues_)
    {
        if (!should_display(queue.name, options))
                continue;
        queue.reader(messages, options);
    }

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
void BridgeInspector<Bridge>::print_header(const std::string& title) const
{
    console_.clear();
    console_.title(title);
    console_.field("Shared memory", shm_name_);
    console_.field("Queues", queues_.size());
    console_.blank();
}



template<typename Bridge>
void BridgeInspector<Bridge>::display(
    const MessageMap& messages,
    const InspectorOptions& options) const
{
    print_header("Bridge Inspector");

    std::size_t displayed_queues = 0;
    std::size_t total_messages = 0;

    for (const auto& queue : queues_)
    {
        if (!should_display(queue.name, options))
            continue;

        ++displayed_queues;

        const auto it = messages.find(queue.name);
        const MessageList* list = nullptr;
        std::size_t count = 0;

        if (it != messages.end())
        {
            list = &it->second;
            count = list->size();
        }

        total_messages += count;
        console_.section(queue.name);
        console_.field("Type", queue.proto_name);
        console_.field("Mode", options.history ? "History" : "Latest");
        console_.field("Messages", count);

        if (!options.verbose)
        {
            console_.blank();
            continue;
        }

        if (count == 0)
        {
            console_.field("Status", "Empty");
            console_.blank();
            continue;
        }

        console_.blank();
        for (std::size_t i = 0; i < list->size(); ++i)
        {
            if (options.history)
            {
                console_.section(queue.name + "/Message #" + std::to_string(i));
            }

            (*list)[i]->PrintDebugString();

            if (i + 1 != list->size())
                console_.blank();
        }

        console_.blank();
    }

    console_.separator('=');
    console_.field("Queues", displayed_queues);
    console_.field("Messages", total_messages);
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
void BridgeInspector<Bridge>::display_yaml(
    const MessageMap& messages,
    const InspectorOptions& options) const
{
    for (const auto& [name, list] : messages)
    {
        std::cout << name << '\n';
        for (const auto& msg : list)
        {
            std::string json;
            google::protobuf::util::MessageToJsonString(*msg, &json);
            YAML::Node node = YAML::Load(json);
            YAML::Emitter out;
            out.SetIndent(4);
            out.SetMapFormat(YAML::Block);
            out.SetSeqFormat(YAML::Block);
            out << node;
            console_.print_indented(out.c_str(), 4);
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
void BridgeInspector<Bridge>::display_progress(
    const MessageMap& messages,
    const InspectorOptions& options) const
{
    print_header("Bridge Progress");
    std::size_t total = 0;

    for (const auto& queue : queues_)
    {
   
        const auto buffered = queue.size();
        total += buffered;

        std::cout
            << std::left
            << std::setw(24) << queue.name
            << '['
            << Console::progress_bar(buffered, queue.capacity, 32)
            << "] "
            << std::setw(3) << buffered
            << '/'
            << queue.capacity
            << '\n';
    }

    std::cout << '\n';

    console_.separator('=');
    console_.field("Buffered messages", total);
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
        else if (options.yaml)
        {
            display_yaml(messages, options);
        }
        else if (options.progress)
        {
            display_progress(messages, options);
        }
        else
        {
            display(messages, options);
        }

        if (!options.once)
            std::this_thread::sleep_for(period);

    } while (!options.once);
}