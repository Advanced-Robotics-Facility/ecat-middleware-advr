#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"
#include "advrf_middleware_core/utils/ecat_discover.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/utils/log.hpp>


namespace middleware_adapter::message {

class AdapterPublishers : public AdapterBase {
public:
  using Pdo = iit::advrf::Ec_slave_pdo;
  using EcatId = std::uint32_t;

  static constexpr std::size_t MaxEcatIds = 256;

  struct CachedPdo {
    bool valid = false;
    bool updated_this_cycle = false;
    EcatId ecat_id = 0;
    Pdo pdo;
  };

  struct ChannelCache {
    std::array<CachedPdo, MaxEcatIds> entries;
    std::vector<EcatId> active_ids;
  };

  using Cache = std::array<ChannelCache, CHANNEL_COUNT>;
  using IdMask = std::bitset<MaxEcatIds>;

  class IPublisher {
  public:
    virtual ~IPublisher() = default;

    virtual void begin_cycle() = 0;
    virtual void consume(const Pdo &pdo) = 0;
    virtual void end_cycle(bool valid) = 0;

    void set_names(std::unordered_map<uint32_t, std::string> m) {
      id_to_name_ = std::move(m);
    }

  protected:
    std::unordered_map<pdo_utils::EcatId, std::string> id_to_name_;
  };

  struct Subscription {
    IPublisher *publisher = nullptr;
    std::vector<ChannelRx> channels;

  private:
    friend class AdapterPublishers;

    IdMask ids_allowed;
    IdMask ids_seen;
    bool accept_all_ids = true;
  };

  AdapterPublishers() {}
  ~AdapterPublishers() override = default;

  ShmRxReader &shm() noexcept { return shm_; }
  const ShmRxReader &shm() const noexcept { return shm_; }

  void spin_once() override {
    fill_cache();
    dispatch();
  }

  bool start() override {
    return shm_.connect(SHM_NRT_RX_PDO, ShmAttachMode::Open);
  }

  bool is_ok() const override { return shm_.is_ok(); }
  void close() override { shm_.close(); }

protected:
  ShmRxReader shm_;

  template <typename PublisherType>
  PublisherType &
  register_publisher(std::vector<ChannelRx> channels,
                     const std::vector<EcatId> &ids_allowed = {}) {
    static_assert(std::is_base_of_v<IPublisher, PublisherType>,
                  "PublisherType must derive from IPublisher");

    auto publisher = std::make_unique<PublisherType>();
    auto *publisher_ptr = publisher.get();

    Subscription subscription;
    subscription.publisher = publisher_ptr;
    subscription.channels = std::move(channels);
    subscription.accept_all_ids = ids_allowed.empty();

    for (const EcatId id : ids_allowed) {
      if (id >= MaxEcatIds) {
        LOG_ERROR("Configured ECAT ID {} exceeds maximum supported ID {}", id,
                  MaxEcatIds - 1);

        continue;
      }

      if (subscription.ids_allowed.test(id)) {
        LOG_ERROR("Duplicate configured ECAT ID {}", id);
        continue;
      }

      subscription.ids_allowed.set(id);
    }

    subscriptions_.push_back(std::move(subscription));
    publishers_.push_back(std::move(publisher));

    return *publisher_ptr;
  }

  // API unit test

  ChannelCache &mutable_channel_cache(ChannelRx channel) noexcept {
    return cache_[channel_index(channel)];
  }

  void dispatch_cached_data() { dispatch(); }

private:

  std::vector<std::unique_ptr<IPublisher>> publishers_;
  std::vector<Subscription> subscriptions_;
  Cache cache_;

  static constexpr std::size_t channel_index(ChannelRx channel) noexcept {
    return static_cast<std::size_t>(channel);
  }

  ChannelCache &channel_cache(ChannelRx channel) noexcept {
    return cache_[channel_index(channel)];
  }

  const ChannelCache &channel_cache(ChannelRx channel) const noexcept {
    return cache_[channel_index(channel)];
  }

  void fill_cache() {
    for (const ChannelRx channel : CHANNELS_ARRAY) {
      const auto device = device_for(channel);

      if (!device) {
        LOG_ERROR("No device mapped for ChannelRx {}",
                  static_cast<int>(channel));
        continue;
      }

      auto &cache = channel_cache(channel);
      std::vector<Pdo> pdos;
      shm_.drain(*device, pdos);

      for(const auto &received : pdos) {
        const int parsed_id = get_ecat_id(received.header().str_id());

        if (parsed_id < 0) {
          LOG_ERROR("Format error for PDO frame with ID {}",
                    received.header().str_id());
          return;
        }

        const auto id = static_cast<EcatId>(parsed_id);

        if (id >= MaxEcatIds) {
          LOG_ERROR("ECAT ID {} exceeds maximum supported ID {}", id,
                    MaxEcatIds - 1);
          return;
        }

        auto &entry = cache.entries[id];

        if (!entry.valid)
          cache.active_ids.push_back(id);

        entry.valid = true;
        entry.ecat_id = id;
        entry.pdo = received;
      }
    }
  }

  void dispatch() {
    for (auto &subscription : subscriptions_) {
      subscription.ids_seen.reset();
      subscription.publisher->begin_cycle();

      for (const ChannelRx channel : subscription.channels) {
        dispatch_cache(channel_cache(channel), subscription);
      }

      const bool valid =
          subscription.accept_all_ids
              ? subscription.ids_seen.any()
              : subscription.ids_seen == subscription.ids_allowed;

      subscription.publisher->end_cycle(valid);
    }
  }

  static void dispatch_cache(const ChannelCache &cache,
                             Subscription &subscription) {
    for (EcatId id : cache.active_ids) {

      const auto &entry = cache.entries[id];

      if (!subscription.accept_all_ids && !subscription.ids_allowed.test(id))
        continue;

      if (subscription.ids_seen.test(id))
        continue;

      subscription.ids_seen.set(id);
      subscription.publisher->consume(entry.pdo);
    }
  }
};

} // namespace middleware_adapter::message