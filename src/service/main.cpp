#include <atomic>
#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>

#include "common/config.hpp"
#include "common/logging.hpp"
#include "service/rcc_service_impl.hpp"
#include "tutorial/tutorial_mode.hpp"

#if __has_include(<systemd/sd-daemon.h>)
#include <systemd/sd-daemon.h>
#define OPENRCC_HAS_SYSTEMD 1
#else
#define OPENRCC_HAS_SYSTEMD 0
static int sd_notify(int unset_environment, const char* state) {
    (void)unset_environment;
    (void)state;
    return 0;
}
#endif

namespace {

std::atomic<bool> g_stop_requested{false};

/**
 * Signal handler requesting graceful shutdown.
 *
 * @param signum POSIX signal number.
 */
void OnSignal(int signum) {
    (void)signum;
    g_stop_requested.store(true);
}

}  // namespace

int main(int argc, char** argv) {
    bool tutorial_mode = false;
    bool non_interactive = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--tutorial") {
            tutorial_mode = true;
        }
        if (arg == "--non-interactive") {
            non_interactive = true;
        }
    }

    if (tutorial_mode) {
        return openrcc::tutorial::RunTutorial(non_interactive);
    }

    // /etc/openrcc/service.toml schema:
    // bind_address = "0.0.0.0:50051"
    // default_tick_rate_hz = 30
    // json_logs = true
    const openrcc::common::ServiceConfig config = openrcc::common::ParseServiceConfig("/etc/openrcc/service.toml");
    openrcc::common::InitLogging(config.json_logs);

    std::signal(SIGINT, OnSignal);
    std::signal(SIGTERM, OnSignal);

    openrcc::service::RccControlServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(config.bind_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    if (server == nullptr) {
        spdlog::error("Failed to start gRPC server at {}", config.bind_address);
        return 1;
    }

    // ARCH: sd_notify ensures systemd only marks us active after the server port is bound and accepting.
    (void)sd_notify(0, "READY=1");
    spdlog::info("OpenRCC service listening on {}", config.bind_address);

    while (!g_stop_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ARCH: Graceful drain gives active jobs a bounded window to finish cleanup before forced shutdown.
    service.DrainAll();
    server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(10));
    server->Wait();

    (void)sd_notify(0, "STOPPING=1");
    spdlog::info("OpenRCC service stopped");
    return 0;
}
