#include "shm_shared_types.hpp"
#include "shm_utils.hpp"
#include "ecat_pdo.pb.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <new>

int main()
{
    SharedMemoryOpenOrCreate shm(SHM_NAME, sizeof(SharedBridge));

    if (!shm.is_valid()) {
        std::cerr << "Failed to open/create shared memory.\n";
        return 1;
    }

    auto* bridge = static_cast<SharedBridge*>(shm.raw_ptr());

    if (shm.created()) {
        new (bridge) SharedBridge();
        std::cout << "Created shared memory.\n";
    }

    ShmProtoHelper proto;
    iit::advrf::Ec_slave_pdo cmd;

    while (true) {

        proto.parse_latest(bridge->cmd, cmd);

        std::cout << "\033[2J\033[H"; // Clear terminal (optional)

        if (cmd.has_cia402_tx_pdo()) {
            const auto& tx = cmd.cia402_tx_pdo();
            std::cout << "target_pos: " << tx.target_pos() << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}