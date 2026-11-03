#!/usr/bin/env bash
set -euo pipefail

if (( EUID != 0 )); then
    echo "Run this script as root." >&2
    exit 1
fi
export DEBIAN_FRONTEND=noninteractive

packages=(
    # Build tools: common middleware packages and all vendor libraries
    build-essential
    cmake
    git

    # advrf_middleware_core
    libspdlog-dev
    libyaml-cpp-dev

    # Master headers consumed by middleware
    libboost-dev

    # advrf_interfaces_protobuf
    protobuf-compiler
    libprotobuf-dev

    # shm_tools
    libncurses-dev

    # advrf_fastdds_lib
    libasio-dev
    libtinyxml2-dev

    # advrf_fastdds_lib and advrf_zenoh_lib
    openjdk-17-jdk

    # advrf_zmq_lib
    pkg-config
    libzmq3-dev

    # advrf_zenoh_lib
    curl
    ca-certificates
)

apt-get update
apt-get install -y --no-install-recommends "${packages[@]}"

# advrf_zenoh_lib
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs |
    sh -s -- -y --default-toolchain stable

cargo_env_line='source "$HOME/.cargo/env"'
if ! grep -Fxq "$cargo_env_line" "$HOME/.bashrc" 2>/dev/null; then
    printf '\n%s\n' "$cargo_env_line" >> "$HOME/.bashrc"
fi
source "$HOME/.cargo/env"

# Note: If Fast-DDS-Gen fails (FastDDS and Zenoh):
# export GRADLE_OPTS="${GRADLE_OPTS:+$GRADLE_OPTS }-Dorg.gradle.daemon=false -Dorg.gradle.parallel=false -Dorg.gradle.workers.max=1"
