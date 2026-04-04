#include "common/config.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace openrcc::common {

/**
 * Trim spaces and quotes from a scalar value.
 *
 * @param value Input value.
 * @return Trimmed scalar.
 */
static std::string TrimScalar(const std::string& value) {
    std::string trimmed = value;
    const std::string whitespace = " \t\r\n\"";
    const std::size_t start = trimmed.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = trimmed.find_last_not_of(whitespace);
    return trimmed.substr(start, end - start + 1);
}

ServiceConfig ParseServiceConfig(const std::string& path) {
    ServiceConfig config;
    std::ifstream input(path);
    if (!input.is_open()) {
        return config;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = TrimScalar(line.substr(0, equals));
        const std::string value = TrimScalar(line.substr(equals + 1));
        if (key == "bind_address") {
            config.bind_address = value;
        } else if (key == "default_tick_rate_hz") {
            config.default_tick_rate_hz = static_cast<std::uint32_t>(std::stoul(value));
        } else if (key == "json_logs") {
            config.json_logs = (value == "true" || value == "1");
        }
    }

    return config;
}

}  // namespace openrcc::common
