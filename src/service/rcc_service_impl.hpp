#pragma once

#include <chrono>
#include <cstdint>

#include <grpcpp/grpcpp.h>

#include "job_manager/job_fsm.hpp"
#include "rcc_control.grpc.pb.h"

namespace openrcc::service {

/**
 * gRPC implementation of canonical OpenRCC control-plane APIs.
 */
class RccControlServiceImpl final : public openrcc::v1::OpenRccControl::Service {
public:
    /**
     * Construct service handler.
     */
    explicit RccControlServiceImpl(std::uint32_t default_tick_rate_hz = 30);

    grpc::Status OpenJob(grpc::ServerContext* context,
                         const openrcc::v1::OpenJobRequest* request,
                         openrcc::v1::OpenJobResponse* response) override;

    grpc::Status ExecuteScript(grpc::ServerContext* context,
                               const openrcc::v1::ExecuteScriptRequest* request,
                               openrcc::v1::ExecuteScriptResponse* response) override;

    grpc::Status CloseJob(grpc::ServerContext* context,
                          const openrcc::v1::CloseJobRequest* request,
                          openrcc::v1::CloseJobResponse* response) override;

    grpc::Status GetJobStatus(grpc::ServerContext* context,
                              const openrcc::v1::JobStatusRequest* request,
                              openrcc::v1::JobStatusResponse* response) override;

    grpc::Status ServerStatus(grpc::ServerContext* context,
                              const openrcc::v1::Empty* request,
                              openrcc::v1::ServerStatusResponse* response) override;

    /**
     * Begin graceful drain for all jobs.
     */
    void DrainAll();

private:
    openrcc::job_manager::JobManager job_manager_;
    std::chrono::steady_clock::time_point started_at_;
    std::uint32_t default_tick_rate_hz_;
};

}  // namespace openrcc::service
