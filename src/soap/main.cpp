#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>

#include "rcc_control.grpc.pb.h"

/**
 * Extract a simple XML tag value.
 *
 * @param xml XML payload.
 * @param tag Tag name without brackets.
 * @return Extracted value or empty string.
 */
static std::string ExtractTag(const std::string& xml, const std::string& tag) {
    const std::string open = "<" + tag + ">";
    const std::string close = "</" + tag + ">";
    const std::size_t begin = xml.find(open);
    const std::size_t end = xml.find(close);
    if (begin == std::string::npos || end == std::string::npos || end <= begin) {
        return "";
    }
    return xml.substr(begin + open.size(), end - begin - open.size());
}

/**
 * Translate SOAP/XML OpenJob envelope to protobuf request.
 *
 * @param xml SOAP XML string.
 * @return OpenJob protobuf request.
 */
static openrcc::v1::OpenJobRequest MapSoapToOpenJob(const std::string& xml) {
    openrcc::v1::OpenJobRequest request;
    request.set_request_id(ExtractTag(xml, "request_id"));
    request.set_job_name(ExtractTag(xml, "job_name"));

    const std::string tick_rate = ExtractTag(xml, "tick_rate_hz");
    if (!tick_rate.empty()) {
        request.set_tick_rate_hz(static_cast<std::uint32_t>(std::stoul(tick_rate)));
    }

    return request;
}

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

    spdlog::info("OpenRCC SOAP shim started (educational skeleton)");
    for (const auto& entry : mapping_table) {
        spdlog::info("SOAP mapping: {} -> {}", entry.first, entry.second);
    }

    const std::string sample_soap =
        "<Envelope><Body><OpenJob><request_id>soap-1</request_id><job_name>soap-job</job_name><tick_rate_hz>30</tick_rate_hz></OpenJob></Body></Envelope>";
    const openrcc::v1::OpenJobRequest open_job = MapSoapToOpenJob(sample_soap);
    spdlog::info("[SOAP→gRPC] Mapped OpenJob SOAP envelope to OpenJobRequest{{request_id={}, job_name={}, tick_rate_hz={}}}",
                 open_job.request_id(), open_job.job_name(), open_job.tick_rate_hz());

    return 0;
}
