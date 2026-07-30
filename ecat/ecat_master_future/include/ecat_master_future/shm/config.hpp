#pragma once

#include <cstddef>
#include <cstdint>

static constexpr const char* SHM_PUB_NAME = "/ecat_master_pub";
static constexpr const char* SHM_REPL_NAME = "/ecat_master_repl";
static constexpr const char* SHM_SUB_NAME = "/ecat_master_sub";

static constexpr size_t PROTO_MAX_BYTES = 512;
static constexpr size_t PROTO_FRAME_HEADER_BYTES = sizeof(uint32_t);
static constexpr size_t MAX_SLAVES_CAPACITY = 64;

static constexpr size_t SHARED_PUB_BRIDGE_SIZE_QUEUE = 64;
static constexpr size_t SHARED_REPL_BRIDGE_SIZE_QUEUE = 16;
static constexpr size_t SHARED_SUB_BRIDGE_SIZE_QUEUE = 16;