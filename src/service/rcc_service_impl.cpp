#include "service/rcc_service_impl.hpp"

#include <chrono>
#include <cstdint>
#include <exception>
#include <string>

#include <spdlog/spdlog.h>

namespace openrcc::service {

namespace {

constexpr std::uint32_t kFallbackTickRateHz = 30;

}  // namespace

RccControlServiceImpl::RccControlServiceImpl(std::uint32_t default_tick_rate_hz)
    : job_manager_(),
      started_at_(std::chrono::steady_clock::now()),
      default_tick_rate_hz_(default_tick_rate_hz == 0 ? kFallbackTickRateHz : default_tick_rate_hz) {}

grpc::Status RccControlServiceImpl::OpenJob(grpc::ServerContext* context,
                                             const openrcc::v1::OpenJobRequest* request,
                                             openrcc::v1::OpenJobResponse* response) {
    (void)context;
    const std::uint32_t tick_rate_hz = request->tick_rate_hz() == 0 ? default_tick_rate_hz_ : request->tick_rate_hz();
    const std::string job_id = job_manager_.OpenJob(tick_rate_hz);

    response->set_request_id(request->request_id());
    response->set_job_id(job_id);
    response->set_state("RUNNING");
    response->set_message("Job opened");
    return grpc::Status::OK;
}

grpc::Status RccControlServiceImpl::ExecuteScript(grpc::ServerContext* context,
                                                   const openrcc::v1::ExecuteScriptRequest* request,
                                                   openrcc::v1::ExecuteScriptResponse* response) {
    (void)context;
    const bool accepted = job_manager_.ExecuteScript(request->job_id(), request->source());
    response->set_request_id(request->request_id());
    response->set_accepted(accepted);
    response->set_message(accepted ? "Script accepted" : "Script rejected");
    return grpc::Status::OK;
}

grpc::Status RccControlServiceImpl::CloseJob(grpc::ServerContext* context,
                                              const openrcc::v1::CloseJobRequest* request,
                                              openrcc::v1::CloseJobResponse* response) {
    (void)context;
    const bool closed = job_manager_.CloseJob(request->job_id(), request->force());
    response->set_request_id(request->request_id());
    response->set_closed(closed);
    if (!closed) {
        response->set_final_state("UNKNOWN_JOB");
        return grpc::Status::OK;
    }

    try {
        response->set_final_state(openrcc::job_manager::ToString(job_manager_.GetJobState(request->job_id())));
    } catch (const std::exception&) {
        response->set_final_state("UNKNOWN_JOB");
    }
    return grpc::Status::OK;
}

grpc::Status RccControlServiceImpl::GetJobStatus(grpc::ServerContext* context,
                                                  const openrcc::v1::JobStatusRequest* request,
                                                  openrcc::v1::JobStatusResponse* response) {
    (void)context;
    try {
        const openrcc::job_manager::JobSnapshot snapshot = job_manager_.GetJobSnapshot(request->job_id());
        response->set_request_id(request->request_id());
        response->set_job_id(request->job_id());
        response->set_state(openrcc::job_manager::ToString(snapshot.state));
        response->set_uptime_ms(snapshot.uptime_ms);
        response->set_memory_bytes(snapshot.memory_bytes);
        return grpc::Status::OK;
    } catch (const std::exception& ex) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, ex.what());
    }
}

grpc::Status RccControlServiceImpl::ServerStatus(grpc::ServerContext* context,
                                                  const openrcc::v1::Empty* request,
                                                  openrcc::v1::ServerStatusResponse* response) {
    (void)context;
    (void)request;

    const auto [total, running, faulted] = job_manager_.AggregateCounts();
    response->set_request_id("server-status");
    response->set_total_jobs(total);
    response->set_running_jobs(running);
    response->set_faulted_jobs(faulted);

    const std::chrono::milliseconds uptime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started_at_);
    response->set_uptime_ms(static_cast<std::uint64_t>(uptime.count()));
    return grpc::Status::OK;
}

void RccControlServiceImpl::DrainAll() {
    job_manager_.DrainAll();
}

}  // namespace openrcc::service
