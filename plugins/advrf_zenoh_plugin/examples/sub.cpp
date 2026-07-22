#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <zenoh.hxx>

namespace
{
volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}
} // namespace

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    const std::string key = argc > 1 ? argv[1] : "advrf/example/simple";

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try
    {
        auto session = zenoh::Session::open(zenoh::Config::create_default());
        auto subscriber = session.declare_subscriber(
            zenoh::KeyExpr(key),
            [](const zenoh::Sample& sample)
            {
                std::cout << "Received '"
                          << sample.get_keyexpr().as_string_view()
                          << "': " << sample.get_payload().as_string() << '\n';
            },
            zenoh::closures::none);

        std::cout << "Subscribed to '" << key << "'. Press Ctrl-C to stop.\n";
        while (running)
        {
            std::this_thread::sleep_for(200ms);
        }
    }
    catch (const zenoh::ZException& error)
    {
        std::cerr << "Zenoh subscriber error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
