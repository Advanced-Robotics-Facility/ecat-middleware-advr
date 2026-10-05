#include "shm_tools/inspector_app.hpp"
#include "shm_tools/read_bridges.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace advrf::middleware::tools::shm {

enum class SourceKind
{
    Rx,
    Tx,
    Service
};

struct SourceSpec
{
    SourceKind kind;
    std::string shm_name;
};

struct CliOptions
{
    std::vector<SourceSpec> sources;
    int rate{10};
    bool history{false};
    bool keep_last_queue{false};
    bool help{false};
};

void print_help(const char* program)
{
    std::cout
        << "Usage:\n"
        << "  " << program << " [sources...] [options]\n\n"
        << "Sources (repeatable):\n"
        << "  --rx  NAME       SharedProtoRxBridge shared memory\n"
        << "  --tx  NAME       SharedProtoTxBridge shared memory\n"
        << "  --service NAME       SharedReplBridge shared memory\n\n"
        << "Options:\n"
        << "  --rate HZ         Refresh rate (default: 10)\n"
        << "  --history         Start in history mode\n"
        << "  -h, --help        Show this help\n\n"
        << "Example:\n"
        << "  " << program
        << " --rx /robot/pub --tx /robot/sub --service /robot/repl --rate 20\n\n"
        << "TUI keys:\n"
        << "  Tab / Shift-Tab   switch shared memory\n"
        << "  1..9              select shared memory directly\n"
        << "  Up/Down or j/k    select queue\n"
        << "  PgUp/PgDn         scroll protobuf detail\n"
        << "  p                  pause\n"
        << "  h                  latest/history\n"
        << "  /                  filter queues\n"
        << "  c                  clear filter\n"
        << "  q                  quit\n";
}

std::string require_value(
    int argc,
    char** argv,
    int& index,
    const std::string& option)
{
    if (index + 1 >= argc)
        throw std::runtime_error("Missing value after " + option);

    ++index;
    return argv[index];
}

CliOptions parse_cli(int argc, char** argv)
{
    CliOptions options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            options.help = true;
        }
        else if (arg == "--rx")
        {
            options.sources.push_back(
                {SourceKind::Rx, require_value(argc, argv, i, arg)});
        }
        else if (arg == "--tx")
        {
            options.sources.push_back(
                {SourceKind::Tx, require_value(argc, argv, i, arg)});
        }
        else if (arg == "--service")
        {
            options.sources.push_back(
                {SourceKind::Service, require_value(argc, argv, i, arg)});
        }
        else if (arg == "--rate")
        {
            const auto value = require_value(argc, argv, i, arg);
            options.rate = std::stoi(value);

            if (options.rate <= 0)
                throw std::runtime_error("--rate must be > 0");
        }
        else if (arg == "--history")
        {
            options.history = true;
        }
        else if (arg == "--keep-last-queue")
        {
            options.keep_last_queue = true;
        }
        else
        {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    return options;
}

std::string make_label(
    SourceKind kind,
    std::size_t occurrence)
{
    std::string base;

    switch (kind)
    {
        case SourceKind::Rx:  base = "RX";  break;
        case SourceKind::Tx:  base = "TX";  break;
        case SourceKind::Service: base = "SRV"; break;
    }

    if (occurrence > 1)
        base += std::to_string(occurrence);

    return base;
}

} // namespace

using namespace advrf::middleware::tools::shm;

int main(int argc, char** argv)
{
    try
    {
        const auto cli = parse_cli(argc, argv);

        if (cli.help)
        {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        if (cli.sources.empty())
        {
            print_help(argv[0]);
            std::cerr << "\nError: configure at least one --rx, --tx or --service source.\n";
            return EXIT_FAILURE;
        }

        InspectorApp app;

        std::size_t pub_count = 0;
        std::size_t sub_count = 0;
        std::size_t repl_count = 0;

        for (const auto& source : cli.sources)
        {
            switch (source.kind)
            {
                case SourceKind::Rx:
                    app.add<ReadBridgeRx>(
                        make_label(SourceKind::Rx, ++pub_count),
                        source.shm_name);
                    break;

                case SourceKind::Tx:
                    app.add<ReadBridgeTx>(
                        make_label(SourceKind::Tx, ++sub_count),
                        source.shm_name);
                    break;

                case SourceKind::Service:
                    app.add<ReadBridgeService>(
                        make_label(SourceKind::Service, ++repl_count),
                        source.shm_name);
                    break;
            }
        }

        InspectorOptions options;
        options.rate = cli.rate;
        options.history = cli.history;
        options.keep_last_queue = cli.keep_last_queue;

        return app.run(options);
    }
    catch (const std::exception& e)
    {
        std::cerr << "shm-inspector: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}