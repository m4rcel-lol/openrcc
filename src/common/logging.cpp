#include "common/logging.hpp"

#include <memory>

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace openrcc::common {

void InitLogging(bool json_mode) {
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> sink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("openrcc", sink);
    if (json_mode) {
        // ARCH: JSON logs are machine-readable to support centralized ingestion and tracing pipelines.
        logger->set_pattern("{\"ts\":\"%Y-%m-%dT%H:%M:%S.%eZ\",\"level\":\"%l\",\"msg\":\"%v\"}");
    } else {
        // ARCH: Human-readable mode helps educational local development and tutorial sessions.
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    }
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
}

}  // namespace openrcc::common
