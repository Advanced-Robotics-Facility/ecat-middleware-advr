#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"

#include <dds/dds.hpp>
#include <sys/types.h>
#include <chrono>

#include "advrf_cyclonedds_plugin/converter.hpp"
#include <ecat_master_future/shm_utils.hpp>
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_middleware_core/utils/log.hpp>



DDSAdapterService::DDSAdapterService(const config::ConfigTopics& config_topics,
                                    dds::domain::DomainParticipant& participant)
    : server_(participant, config_topics.replCmd.request(), config_topics.replCmd.reply())
{
    server_.set_callback([this](const RequestDDS& req) {
        return process_request_dds(req);
    });

}

ResponseDDS DDSAdapterService::process_request_dds(const RequestDDS& request)
{
    RequestProtobuf pb_req;
    convert::protobuf::from_dds(request, pb_req);
    ResponseProtobuf pb_resp = this->process_request(pb_req);
    ResponseDDS resp;
    convert::dds::from_protobuf(pb_resp, resp);
    resp.request_id() = request.request_id();
    return resp;
}