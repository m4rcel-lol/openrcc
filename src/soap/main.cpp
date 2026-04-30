#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>

#include "rcc_control.grpc.pb.h"
#include "soap/soap_bridge.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const std::unordered_map<std::string, std::string> mapping_table = {
        {"OpenJob", "openrcc.v1.OpenJobRequest"},
        {"ExecuteScript", "openrcc.v1.ExecuteScriptRequest"},
        {"CloseJob", "openrcc.v1.CloseJobRequest"},
        {"GetJobStatus", "openrcc.v1.JobStatusRequest"},
        {"ServerStatus", "openrcc.v1.Empty"},
    };

    // ARCH: SOAP bridge forwards to local gRPC service to preserve one canonical business logic path.
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<openrcc::v1::OpenRccControl::Stub> stub = openrcc::v1::OpenRccControl::NewStub(channel);
    if (stub == nullptr) {
        spdlog::error("Failed to create OpenRCC gRPC stub");
        return 1;
    }

    spdlog::info("OpenRCC SOAP shim started (educational skeleton)");
    for (const auto& entry : mapping_table) {
        spdlog::info("SOAP mapping: {} -> {}", entry.first, entry.second);
    }

    const std::string sample_soap =
        "<Envelope><Body><OpenJob><request_id>soap-1</request_id><job_name>soap-job</job_name><tick_rate_hz>30</tick_rate_hz></OpenJob></Body></Envelope>";
    const openrcc::v1::OpenJobRequest open_job = openrcc::soap::MapSoapToOpenJob(sample_soap);
    spdlog::info("[SOAP->gRPC] Mapped OpenJob SOAP envelope to OpenJobRequest{{request_id={}, job_name={}, tick_rate_hz={}}}",
                 open_job.request_id(), open_job.job_name(), open_job.tick_rate_hz());

    return 0;
}
