#pragma once

#include <cstdint>
#include <string>

namespace openrcc::common {

/**
 * Runtime configuration for OpenRCC service.
 */
struct ServiceConfig {
    std::string bind_address = "0.0.0.0:50051";
    std::uint32_t default_tick_rate_hz = 30;
    bool json_logs = true;
};

/**
 * Parse service configuration from a TOML-like file.
 *
 * @param path Absolute path to configuration file.
 * @return Parsed service configuration with defaults on missing fields.
 */
ServiceConfig ParseServiceConfig(const std::string& path);

}  // namespace openrcc::common
