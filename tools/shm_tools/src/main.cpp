#include "shm_tools/inspector_app.hpp"
#include "shm_tools/read_bridges.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

enum class SourceKind
{
    Pub,
    Sub,
    Repl
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
    bool help{false};
};

void print_help(const char* program)
{
    std::cout
        << "Usage:\n"
        << "  " << program << " [sources...] [options]\n\n"
        << "Sources (repeatable):\n"
        << "  --pub  NAME       SharedProtoPubBridge shared memory\n"
        << "  --sub  NAME       SharedProtoSubBridge shared memory\n"
        << "  --repl NAME       SharedReplBridge shared memory\n\n"
        << "Options:\n"
        << "  --rate HZ         Refresh rate (default: 10)\n"
        << "  --history         Start in history mode\n"
        << "  -h, --help        Show this help\n\n"
        << "Example:\n"
        << "  " << program
        << " --pub /robot/pub --sub /robot/sub --repl /robot/repl --rate 20\n\n"
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
        else if (arg == "--pub")
        {
            options.sources.push_back(
                {SourceKind::Pub, require_value(argc, argv, i, arg)});
        }
        else if (arg == "--sub")
        {
            options.sources.push_back(
                {SourceKind::Sub, require_value(argc, argv, i, arg)});
        }
        else if (arg == "--repl")
        {
            options.sources.push_back(
                {SourceKind::Repl, require_value(argc, argv, i, arg)});
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
        case SourceKind::Pub:  base = "PUB";  break;
        case SourceKind::Sub:  base = "SUB";  break;
        case SourceKind::Repl: base = "REPL"; break;
    }

    if (occurrence > 1)
        base += std::to_string(occurrence);

    return base;
}

} // namespace

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
            std::cerr << "\nError: configure at least one --pub, --sub or --repl source.\n";
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
                case SourceKind::Pub:
                    app.add<ReadBridgePub>(
                        make_label(SourceKind::Pub, ++pub_count),
                        source.shm_name);
                    break;

                case SourceKind::Sub:
                    app.add<ReadBridgeSub>(
                        make_label(SourceKind::Sub, ++sub_count),
                        source.shm_name);
                    break;

                case SourceKind::Repl:
                    app.add<ReadBridgeRepl>(
                        make_label(SourceKind::Repl, ++repl_count),
                        source.shm_name);
                    break;
            }
        }

        InspectorOptions options;
        options.rate = cli.rate;
        options.history = cli.history;

        return app.run(options);
    }
    catch (const std::exception& e)
    {
        std::cerr << "shm-inspector: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}