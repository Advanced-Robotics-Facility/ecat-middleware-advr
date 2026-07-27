#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "advrf_middleware_core/adapters/adapter_publishers.hpp"

using AdapterPublishers =
    middleware_adapter::message::AdapterPublishers;

using Pdo = AdapterPublishers::Pdo;
using EcatId = AdapterPublishers::EcatId;
// using Channel = middleware_adapter::Channel;


/**
 * Publisher minimal utilisé pour vérifier le comportement du dispatcher.
 */
class TestPublisher : public AdapterPublishers::IPublisher
{
public:
    void begin_cycle() override
    {
        ++begin_count;
        consumed_ids.clear();
    }

    void consume(const Pdo& pdo) override
    {
        ++consume_count;

        const auto& id = pdo.header().str_id();
        consumed_names.push_back(id);
    }

    void end_cycle(bool valid) override
    {
        ++end_count;
        last_valid = valid;
    }

    std::size_t begin_count{0};
    std::size_t consume_count{0};
    std::size_t end_count{0};

    bool last_valid{false};

    std::vector<EcatId> consumed_ids;
    std::vector<std::string> consumed_names;
};


/**
 * Rend accessibles les méthodes protégées ajoutées pour les tests.
 */
class TestAdapterPublishers : public AdapterPublishers
{
public:
    using AdapterPublishers::dispatch_cached_data;
    using AdapterPublishers::mutable_channel_cache;
    using AdapterPublishers::register_publisher;
};


void clear_cache(
    TestAdapterPublishers& adapter,
    Channel channel)
{
    auto& cache = adapter.mutable_channel_cache(channel);
    cache.valid_count = 0;
}


Pdo& add_cached_pdo(
    TestAdapterPublishers& adapter,
    Channel channel,
    EcatId ecat_id,
    const std::string& component_name)
{
    auto& cache = adapter.mutable_channel_cache(channel);

    if (cache.valid_count == cache.storage.size()) {
        cache.storage.emplace_back();
    }

    auto& cached = cache.storage[cache.valid_count++];

    cached.ecat_id = ecat_id;
    cached.pdo.Clear();
    cached.pdo.mutable_header()->set_str_id(component_name);

    return cached.pdo;
}


void test_empty_cache_is_invalid()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Imu},
            {1});

    adapter.dispatch_cached_data();

    assert(publisher.begin_count == 1);
    assert(publisher.consume_count == 0);
    assert(publisher.end_count == 1);
    assert(!publisher.last_valid);
}


void test_allowed_id_is_dispatched()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Imu},
            {1});

    add_cached_pdo(
        adapter,
        Channel::Imu,
        1,
        "imu_1");

    adapter.dispatch_cached_data();

    assert(publisher.begin_count == 1);
    assert(publisher.consume_count == 1);
    assert(publisher.end_count == 1);
    assert(publisher.last_valid);

    assert(publisher.consumed_names.size() == 1);
    assert(publisher.consumed_names.front() == "imu_1");
}


void test_disallowed_id_is_ignored()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Imu},
            {1});

    add_cached_pdo(
        adapter,
        Channel::Imu,
        2,
        "imu_2");

    adapter.dispatch_cached_data();

    assert(publisher.begin_count == 1);
    assert(publisher.consume_count == 0);
    assert(publisher.end_count == 1);
    assert(!publisher.last_valid);
}


void test_missing_expected_id_is_invalid()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2, 3, 4});

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2");

    add_cached_pdo(
        adapter,
        Channel::Motor,
        3,
        "motor_3");

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 2);
    assert(!publisher.last_valid);
}


void test_all_expected_ids_are_valid()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2, 3, 4});

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2");

    add_cached_pdo(
        adapter,
        Channel::Motor,
        3,
        "motor_3");

    add_cached_pdo(
        adapter,
        Channel::Motor,
        4,
        "motor_4");

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 3);
    assert(publisher.last_valid);
}


void test_empty_allowed_ids_accepts_any_id()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::PowerBoard});

    add_cached_pdo(
        adapter,
        Channel::PowerBoard,
        42,
        "power_board_42");

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 1);
    assert(publisher.last_valid);
}


void test_accept_all_without_data_is_invalid()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::PowerBoard});

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 0);
    assert(!publisher.last_valid);
}


void test_same_pdo_is_dispatched_to_multiple_publishers()
{
    TestAdapterPublishers adapter;

    auto& first =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2});

    auto& second =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2});

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2");

    adapter.dispatch_cached_data();

    assert(first.consume_count == 1);
    assert(second.consume_count == 1);

    assert(first.last_valid);
    assert(second.last_valid);
}


void test_publisher_can_subscribe_to_multiple_channels()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {
                Channel::Motor,
                Channel::Gripper
            },
            {
                2,
                20
            });

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2");

    add_cached_pdo(
        adapter,
        Channel::Gripper,
        20,
        "gripper_20");

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 2);
    assert(publisher.last_valid);
}


void test_duplicate_id_is_consumed_once()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2});

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2_first");

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2_second");

    adapter.dispatch_cached_data();

    assert(publisher.consume_count == 1);
    assert(publisher.last_valid);

    // Avec le parcours actuel, la première frame est conservée.
    assert(publisher.consumed_names.size() == 1);
    assert(publisher.consumed_names.front() == "motor_2_first");
}


void test_seen_ids_are_reset_between_cycles()
{
    TestAdapterPublishers adapter;

    auto& publisher =
        adapter.register_publisher<TestPublisher>(
            {Channel::Motor},
            {2});

    add_cached_pdo(
        adapter,
        Channel::Motor,
        2,
        "motor_2");

    adapter.dispatch_cached_data();

    assert(publisher.begin_count == 1);
    assert(publisher.consume_count == 1);
    assert(publisher.end_count == 1);
    assert(publisher.last_valid);

    clear_cache(
        adapter,
        Channel::Motor);

    adapter.dispatch_cached_data();

    assert(publisher.begin_count == 2);
    assert(publisher.consume_count == 1);
    assert(publisher.end_count == 2);
    assert(!publisher.last_valid);
}


void test_channel_cache_storage_is_reused()
{
    TestAdapterPublishers adapter;

    auto& cache =
        adapter.mutable_channel_cache(Channel::Motor);

    cache.storage.resize(10);
    cache.valid_count = 10;

    const auto previous_size = cache.storage.size();
    const auto previous_capacity = cache.storage.capacity();
    const auto* previous_data = cache.storage.data();

    cache.valid_count = 0;

    assert(cache.storage.size() == previous_size);
    assert(cache.storage.capacity() == previous_capacity);
    assert(cache.storage.data() == previous_data);
}


int main()
{
    test_empty_cache_is_invalid();
    test_allowed_id_is_dispatched();
    test_disallowed_id_is_ignored();
    test_missing_expected_id_is_invalid();
    test_all_expected_ids_are_valid();
    test_empty_allowed_ids_accepts_any_id();
    test_accept_all_without_data_is_invalid();
    test_same_pdo_is_dispatched_to_multiple_publishers();
    test_publisher_can_subscribe_to_multiple_channels();
    test_duplicate_id_is_consumed_once();
    test_seen_ids_are_reset_between_cycles();
    test_channel_cache_storage_is_reused();

    std::cout
        << "All AdapterPublishers tests passed."
        << std::endl;

    return 0;
}