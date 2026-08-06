#include "shm_tools/cli.hpp"
#include "shm_tools/bridges.hpp"

int main(int argc, char** argv)
{
    auto args = parse_arguments(argc, argv);

    switch (args.bridge)
    {
        case BridgeType::Pub:
            ReadBridgePub(SHM_NRT_RX_PDO).run(args.options);
            break;

        case BridgeType::Sub:
            ReadBridgeSub(SHM_TX_PDO).run(args.options);
            break;

        case BridgeType::Repl:
            ReadBridgeRepl(SHM_REPL_NAME).run(args.options);
            break;
    }

    return 0;
}