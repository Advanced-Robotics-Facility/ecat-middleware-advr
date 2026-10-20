#include <fstream>
#include <vector>
#include <iostream>

#include "advrf_middleware_core/config/config_topics.hpp"
#include "advrf_middleware_core/config/runtime_config.hpp"
#include "advrf_middleware_core/test/config_test_support.hpp"

using namespace advrf::middleware::config;
int main() {
    try {
        const auto root = YAML::Load(R"(
robot:
  name: kyon
  namespace: advrf
  joint_map: {1: lf_haa_joint, 4: imu}
runtime: {publishers_hz: 500, subscribers_hz: 200, service_hz: 20}
)");
        auto cfg = parse_common_config({root, "/tmp/config.yaml"});
        validate(cfg);
        require(cfg.robot.robot_name == "kyon" && cfg.robot.map_ecat_id.at(4) == "imu", "robot identity/map");
        require(cfg.runtime.rate_publishers == 500 && cfg.runtime.rate_subscribers == 200, "YAML runtime");
        CommonOptions cli;
        require(cli.consume("--rate_publishers", "1000"), "legacy CLI flag");
        require(cli.consume("--rate-subscribers", "250"), "new CLI flag");
        cli.runtime.apply(cfg.runtime);
        validate(cfg);
        require(cfg.runtime.rate_publishers == 1000 && cfg.runtime.rate_subscribers == 250 && cfg.runtime.rate_service == 20,
                "only explicitly provided CLI overrides apply");
        require(period_from_rate(1000).count() == 1000, "rate conversion");
        require(ConfigTopics({"", "kyon"}).rx.motor() == "rt/kyon/rx/motors", "empty namespace topic");
        expect_error([] { parse_runtime_config(YAML::Load("runtime: {publisher_hz: 500}")); }, "runtime.publisher_hz");
        expect_error([&] { cli.consume("--rate-publishers", "-1"); }, "--rate-publishers");
        expect_error([&] { cli.consume("--rate-publishers", "1000junk"); }, "--rate-publishers");
        expect_error([&] { cli.consume("--rate-publishers", "4294967296"); }, "--rate-publishers");
        expect_error([] { validate(RuntimeConfig{0, 100, 10}); }, "runtime.publishers_hz");
        expect_error([] { validate(RuntimeConfig{1000001, 100, 10}); }, "runtime.publishers_hz");
        const auto legacy = parse_robot_config(YAML::Load("dds: {robot_name: old, namespace: advrf}\njoint_map: {2: knee}"));
        require(legacy.robot_name == "old" && legacy.map_ecat_id.at(2) == "knee", "legacy YAML");
        expect_error([] { parse_robot_config(YAML::Load("robot: {name: kyon, joint_map: {1: [invalid]}}")); }, "robot.joint_map");
        expect_error([] { parse_robot_config(YAML::Load("robot: {name: kyon, joint_map: {-1: knee}}")); }, "robot.joint_map");
        expect_error([] { parse_robot_config(YAML::Load("robot: {name: '', joint_map: {}}")); }, "robot.name");
        expect_error([] { parse_robot_config(YAML::Load("robot: {name: kyon}")); }, "robot.joint_map");
        expect_error([] { parse_runtime_config(YAML::Load("runtime: [10, 20]")); }, "runtime");
        expect_error([] { parse_runtime_config(YAML::Load("runtime: {publishers_hz: nope}")); }, "runtime.publishers_hz");
        require(!load_robot_config("/nonexistent/advrf/map.yaml", "/nonexistent/advrf/config.yaml"), "failed legacy load must return nullopt");
        const ConfigFile file{root, "/tmp/robot/middleware/config.yaml"};
        require(file.resolve("../zenoh.json5") == "/tmp/robot/zenoh.json5", "YAML relative references");
        require(file.resolve("/tmp/absolute.json5") == "/tmp/absolute.json5", "absolute references");
        // File validation and builder lifetime are tested with an owned temporary directory.
        const auto dir = std::filesystem::temp_directory_path() /
            ("advrf-config-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directory(dir);
        struct Cleanup { std::filesystem::path path; ~Cleanup() { std::filesystem::remove_all(path); } } cleanup{dir};
        const auto* previous_env = std::getenv("EC_CFG");
        const std::optional<std::string> previous = previous_env ? std::optional<std::string>(previous_env) : std::nullopt;
        struct RestoreEnv {
            std::optional<std::string> value;
            ~RestoreEnv() { if (value) setenv("EC_CFG", value->c_str(), 1); else unsetenv("EC_CFG"); }
        } restore{previous};
        unsetenv("EC_CFG");
        expect_error([] { middleware_config_path(); }, "EC_CFG");
        setenv("EC_CFG", "", 1);
        expect_error([] { middleware_config_path(); }, "EC_CFG");
        setenv("EC_CFG", (dir / "missing.yaml").c_str(), 1);
        expect_error([] { middleware_config_path(); }, "EC_CFG");
        const auto ec_cfg = dir / "ec_cfg.yaml";
        setenv("EC_CFG", ec_cfg.c_str(), 1);
        for (const auto& folder : std::vector<std::string>{"${PWD}/middleware/", "$PWD/middleware/", "middleware/", (dir / "middleware").string()}) {
            std::ofstream(ec_cfg) << "robot: {robot_middleware_folder: '" << folder << "'}\n";
            require(middleware_config_path() == dir / "middleware/config.yaml", "runtime middleware folder");
        }
        const auto link = dir / "alias.yaml";
        std::filesystem::create_symlink(ec_cfg, link);
        setenv("EC_CFG", link.c_str(), 1);
        require(middleware_config_path() == dir / "middleware/config.yaml", "symlink target directory");
        std::ofstream(ec_cfg) << "robot: {}\n";
        expect_error([] { middleware_config_path(); }, "robot.robot_middleware_folder");
        const auto map_file = dir / "robot_id_map.yaml";
        std::ofstream(map_file) << "joint_map: {7: external_joint}\n";
        std::ofstream(ec_cfg) << "robot: {robot_id_map_path: '${PWD}/robot_id_map.yaml'}\n";
        auto external = parse_common_config({root, dir / "middleware/config.yaml"});
        require(external.robot.robot_name == "kyon" && external.robot.ns == "advrf", "middleware identity retained");
        require(external.robot.map_ecat_id.size() == 1 && external.robot.map_ecat_id.at(7) == "external_joint",
                "EC_CFG map replaces middleware map");
        const auto no_map = YAML::Load("robot: {name: pegasus, namespace: advrf}");
        require(parse_common_config({no_map, dir / "middleware/config.yaml"}).robot.map_ecat_id.at(7) == "external_joint",
                "middleware joint_map is optional with EC_CFG");
        std::ofstream(map_file) << "joint_map: {-1: invalid}\n";
        expect_error([&] { parse_common_config({root, dir / "middleware/config.yaml"}); }, "joint_map");
        std::filesystem::remove(map_file);
        expect_error([&] { parse_common_config({root, dir / "middleware/config.yaml"}); }, "robot_id_map.yaml");
        std::ofstream(ec_cfg) << "robot: {}\n";
        expect_error([&] { parse_common_config({root, dir / "middleware/config.yaml"}); }, "robot.robot_id_map_path");
        unsetenv("EC_CFG");
        const auto path = dir / "config.yaml";
        std::ofstream(path) << "version: 1\nrobot: {name: kyon, namespace: advrf, joint_map: {1: knee}}\n";
        const auto loaded = ConfigFile::load(path);
        require(parse_common_config(loaded).robot.robot_name == "kyon", "file load");
        const auto built = RobotConfigBuilder().from_ecat_config(path.string()).from_robot_id_map(path.string()).build();
        require(built.map_ecat_id.at(1) == "knee", "builder owns result after temporary destroyed");
        std::ofstream(path) << "version: 2\n";
        expect_error([&] { ConfigFile::load(path); }, "config.version");
        std::ofstream(path) << "robot: [\n";
        expect_error([&] { ConfigFile::load(path); }, path.string());
        std::ofstream(path) << "[]\n";
        expect_error([&] { ConfigFile::load(path); }, "mapping");
        std::cout << "Common configuration tests passed\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
