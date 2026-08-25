#pragma once
#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

struct PdoMetadata {
  uint32_t ecat_id;
  std::string name;
  iit::advrf::Ec_slave_pdo::Type type;
  ChannelRx channel;
  DeviceTypeRx device;
};

class PdoDiscover {
public:
  using Pdo = iit::advrf::Ec_slave_pdo;
  using PdoMap = std::map<uint32_t, PdoMetadata>;

  PdoDiscover() = default;
  ~PdoDiscover() = default;

  bool start(const std::string &shm_name) {
    return shm_.connect(shm_name, ShmAttachMode::Open);
  }

  PdoMap discover(const std::set<uint32_t> &pdo_ids) {
    PdoMap pdo_map;

    while (pdo_map.size() < pdo_ids.size()) {
      discover_once(pdo_ids, pdo_map);
      LOG_INFO("Pdo discovery in progress, found {}/{} PDOs", pdo_map.size(), pdo_ids.size());
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return pdo_map;
  }

  bool is_ok() const { return shm_.is_ok(); }
  void close() { shm_.close(); }

private:

  void discover_once(const std::set<uint32_t> &pdo_ids, PdoMap &pdo_map) {
    for (const auto channel : CHANNELS_ARRAY) {
      const auto device = device_for(channel);

      if (!device) {
        LOG_ERROR("Failed to get device for channel {}",
                  static_cast<int>(channel));
        continue;
      }

      std::vector<Pdo> pdos;
      shm_.peek_all(*device, pdos);

      for (const auto &pdo : pdos) {
        const auto &str_id = pdo.header().str_id();
        const int parsed_id = get_ecat_id(str_id);

        if (parsed_id < 0) {
          LOG_ERROR("Format error for PDO frame with ID {}", str_id);
          continue;
        }

        const auto id = static_cast<uint32_t>(parsed_id);

        if (pdo_ids.find(id) == pdo_ids.end() ||
            pdo_map.find(id) != pdo_map.end()) {
          continue;
        }

        pdo_map.emplace(id, PdoMetadata{
                                .ecat_id = id,
                                .name = str_id,
                                .type = pdo.type(),
                                .channel = channel,
                                .device = *device,
                            });
      }
    }
  }

  ShmRxReader shm_;
};