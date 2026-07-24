#include <advrf_middleware_core/plugin/plugin_exec.hpp>
#include <advrf_middleware_core/robot_config.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    advrf::plugin::PluginExec plugin_exec;

    auto cfg = load_robot_config(ROBOT_CONFIG_DIR);
    if (!cfg) return 1;
    
    auto dds_participant = dds::domain::DomainParticipant(cfg->domain_id);
    auto config = config::ConfigTopics({"advrf", "robot"});

    // service
    std::shared_ptr<DDSAdapterService> dds_adapter_service = std::make_shared<DDSAdapterService>();
    dds_adapter_service->init(config, dds_participant);    
    dds_adapter_service->shm().connect(SHM_REPL_NAME);

    // publishers
    std::shared_ptr<DDSAdapterPublishers> dds_adapter_publishers = std::make_shared<DDSAdapterPublishers>();
    dds_adapter_publishers->shm().connect(SHM_NAME);
    if (!dds_adapter_publishers->init(config, dds_participant)) {
        LOG_ERROR("Failed to bind to target DDS channels.");
        return 1;
    }

    plugin_exec.register_adapter({
            dds_adapter_service,
            std::chrono::microseconds(100 * 1000)
    });

    plugin_exec.register_adapter({
            dds_adapter_publishers, 
            std::chrono::microseconds(1000)
        });

    LOG_INFO("Start");
    plugin_exec.start();

    return 0;
}