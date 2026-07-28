#pragma once

#include "shm_tools/bridge_inspector.hpp"

#include <getopt.h>
#include <iostream>
#include <string>

enum class BridgeType
{
    Pub,
    Sub,
    Repl
};

struct CLIArguments
{
    BridgeType bridge = BridgeType::Pub;
    InspectorOptions options;
};

inline void usage(const char* argv0)
{
    std::cout << R"(

Bridge Inspector

Usage:
    )" << argv0 << R"( <bridge> [options]

Bridges

    pub
    sub
    repl

Options

    --once             Read once then exit
    --quiet            Disable verbose output
    --json             Dump protobuf as JSON
    --stats            Statistics mode
    --history          Display every buffered message
    --rate <Hz>        Refresh rate (default 10)
    --key <name>       Filter queue (repeatable)

Examples

    bridge_inspector pub-bridge
    bridge_inspector pub-bridge --once
    bridge_inspector pub-bridge --history
    bridge_inspector pub-bridge --history --key imu
    bridge_inspector pub-bridge --rate 100
    bridge_inspector pub-bridge --key imu
    bridge_inspector pub-bridge --key imu --key motor
    bridge_inspector repl-bridge --json

)";
}

inline CLIArguments parse_arguments(int argc, char** argv)
{
    if (argc < 2)
    {
        usage(argv[0]);
        std::exit(EXIT_FAILURE);
    }

    CLIArguments args;

    std::string bridge = argv[1];

    if (bridge == "pub-bridge")
        args.bridge = BridgeType::Pub;
    else if (bridge == "sub-bridge")
        args.bridge = BridgeType::Sub;
    else if (bridge == "repl-bridge")
        args.bridge = BridgeType::Repl;
    else
    {
        std::cerr << "Unknown bridge: " << bridge << '\n';
        std::exit(EXIT_FAILURE);
    }

    optind = 2;

    static option long_options[] =
    {
        {"once",    no_argument,       nullptr, 'o'},
        {"quiet",   no_argument,       nullptr, 'q'},
        {"json",    no_argument,       nullptr, 'j'},
        {"yaml",    no_argument,       nullptr, 'y'},
        {"stats",   no_argument,       nullptr, 's'},
        {"progress",   no_argument,       nullptr, 'p'},
        {"history", no_argument,       nullptr, 'h'},
        {"rate",    required_argument, nullptr, 'r'},
        {"key",     required_argument, nullptr, 'k'},
        {nullptr,   0,                 nullptr,  0 }
    };

    int c;

    while ((c = getopt_long(
        argc,
        argv,
        "",
        long_options,
        nullptr)) != -1)
    {
        switch (c)
        {
        case 'o':
            args.options.once = true;
            break;

        case 'q':
            args.options.verbose = false;
            break;

        case 'j':
            args.options.json = true;
            break;

        case 's':
            args.options.stats = true;
            break;

        case 'h':
            args.options.history = true;
            break;

        case 'r':
            args.options.rate = std::max(1, std::stoi(optarg));
            break;

        case 'k':
            args.options.filter.emplace_back(optarg);
            break;

        case 'y':
            args.options.yaml = true;
            break;

        case 'p':
            args.options.progress = true;
            break;

        default:
            usage(argv[0]);
            std::exit(EXIT_FAILURE);
        }
    }

    if (args.options.json && args.options.stats)
    {
        std::cerr << "--json and --stats cannot be used together.\n";
        std::exit(EXIT_FAILURE);
    }

    return args;
}