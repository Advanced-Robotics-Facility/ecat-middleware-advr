#pragma once

#include "shm_tools/inspector_types.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <google/protobuf/message.h>

#include <shm_types.hpp>
#include <shm_utils.hpp>

class IInspectorSource {
public:
  virtual ~IInspectorSource() = default;

  virtual InspectorSnapshot poll(bool history, bool keep_last_queue) = 0;
  virtual std::string_view shm_name() const noexcept = 0;
  virtual void reconnect() = 0;
};

template <typename Bridge> class BridgeInspector : public IInspectorSource {
public:
  using MessagePtr = std::unique_ptr<google::protobuf::Message>;
  using MessageList = std::vector<MessagePtr>;
  using MessageMap = std::unordered_map<std::string, MessageList>;

  explicit BridgeInspector(std::string shm_name)
      : shm_name_(std::move(shm_name)),
        shm_(SharedMemory<Bridge>::open(shm_name_)) {}

  ~BridgeInspector() override = default;

  InspectorSnapshot poll(bool history, bool keep_last_queue) override {
    initialize();

    auto messages = read(history);
    update_stats(messages);

    return make_snapshot(messages, history, keep_last_queue);
  }



    std::string_view shm_name() const noexcept override { return shm_name_; }

    void reconnect() override
    {
        queues_.clear();
        stats_.clear();
        bridge_ = nullptr;
        initialized_ = false;
        shm_.reset();
        shm_ = SharedMemory<Bridge>::open(shm_name_);
        initialize();
    }

protected:
  virtual void declare() = 0;

  template <typename Proto, typename Queue>
  void register_queue(const std::string &name, Queue &queue) {
    queues_.push_back(
        {name, Proto::descriptor()->full_name(), Queue::capacity,

         [name, &queue](MessageMap &messages, bool history) {
           if (history) {
             Proto proto;

             ShmProtoHelper::peek_all(queue, proto, [&](const Proto &p) {
               messages[name].push_back(std::make_unique<Proto>(p));
             });

             return;
           }

           auto msg = std::make_unique<Proto>();

           if (ShmProtoHelper::peek_latest(queue, *msg)) {
             messages[name].push_back(std::move(msg));
           }
         },

         [&queue]() -> std::size_t {
           return static_cast<std::size_t>(queue.size());
         }});
  }

  Bridge *bridge_{nullptr};

private:
  struct QueueDescriptor {
    std::string name;
    std::string proto_name;
    std::size_t capacity{0};
    std::function<void(MessageMap &, bool)> reader;
    std::function<std::size_t()> size;
  };

  struct QueueStats {
    std::size_t total_observed{0};
    std::chrono::steady_clock::time_point first_seen{};
    std::chrono::steady_clock::time_point last_seen{};
    bool initialized{false};
  };

  void initialize() {
    if (initialized_)
      return;

    if (!shm_ || !shm_->is_ok()) {
      throw std::runtime_error("Unable to open shared memory '" + shm_name_ +
                               "'");
    }

    bridge_ = shm_->get();

    if (!bridge_) {
      throw std::runtime_error("Shared memory '" + shm_name_ +
                               "' returned a null bridge");
    }
    queues_.clear();
    declare();

    initialized_ = true;
  }

  MessageMap read(bool history) {
    MessageMap messages;

    for (const auto &queue : queues_)
      queue.reader(messages, history);

    return messages;
  }

  void update_stats(const MessageMap &messages) {
    const auto now = std::chrono::steady_clock::now();

    for (const auto &[name, list] : messages) {
      if (list.empty())
        continue;

      auto &stats = stats_[name];

      if (!stats.initialized) {
        stats.initialized = true;
        stats.first_seen = now;
      }

      stats.last_seen = now;
      stats.total_observed += list.size();
    }
  }

  InspectorSnapshot make_snapshot(const MessageMap &messages,
                                  bool history, bool keep_last_queue) const {
    using namespace std::chrono;

    InspectorSnapshot snapshot;
    snapshot.shm_name = shm_name_;
    snapshot.history = history;
    snapshot.keep_last_queue = keep_last_queue;

    const auto now = steady_clock::now();

    snapshot.queues.reserve(queues_.size());

    for (const auto &queue : queues_) {
      QueueSnapshot item;

      item.name = queue.name;
      item.proto_name = queue.proto_name;
      item.buffered = queue.size();
      item.capacity = queue.capacity;

      snapshot.total_buffered += item.buffered;

      if (const auto it = messages.find(queue.name); it != messages.end()) {
        item.messages.reserve(it->second.size());

        for (const auto &message : it->second) {
          item.messages.push_back(message->DebugString());
        }
      }

      if (const auto it = stats_.find(queue.name);
          it != stats_.end() && it->second.initialized) {
        const auto &stats = it->second;

        item.observed_messages = stats.total_observed;

        const double elapsed = duration<double>(now - stats.first_seen).count();

        item.observed_rate =
            elapsed > 0.0 ? static_cast<double>(stats.total_observed) / elapsed
                          : 0.0;

        item.age_ms =
            duration_cast<milliseconds>(now - stats.last_seen).count();
      }

      snapshot.queues.push_back(std::move(item));
    }

    return snapshot;
  }

  bool initialized_{false};
  std::vector<QueueDescriptor> queues_;
  std::unordered_map<std::string, QueueStats> stats_;
  std::string shm_name_;
  std::unique_ptr<SharedMemory<Bridge>> shm_;
};
