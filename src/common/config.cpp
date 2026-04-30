#include "common/config.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace openrcc::common {
namespace {

/**
 * Trim ASCII whitespace from both ends of a string view.
 *
 * @param value Input value.
 * @return Trimmed view.
 */
std::string_view Trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return value;
}

/**
 * Remove inline comments while preserving # inside quoted strings.
 *
 * @param line Raw config line.
 * @return Line without an unquoted trailing comment.
 */
std::string_view StripInlineComment(std::string_view line) {
    bool in_quotes = false;
    bool escaped = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (escaped) {
            escaped = false;
            continue;
        }
        if (ch == '\\' && in_quotes) {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (ch == '#' && !in_quotes) {
            return line.substr(0, i);
        }
    }
    return line;
}

/**
 * Trim spaces and one balanced pair of quotes from a scalar value.
 *
 * @param value Input value.
 * @return Trimmed scalar.
 */
std::string TrimScalar(std::string_view value) {
    value = Trim(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value.remove_prefix(1);
        value.remove_suffix(1);
    }
    return std::string(value);
}

std::string ConfigError(const std::string& path, std::size_t line_number, const std::string& message) {
    return path + ":" + std::to_string(line_number) + ": " + message;
}

std::uint32_t ParseUint32Field(const std::string& path,
                               std::size_t line_number,
                               std::string_view raw_value,
                               const std::string& field_name) {
    const std::string value = std::string(Trim(raw_value));
    if (value.empty()) {
        throw std::runtime_error(ConfigError(path, line_number, field_name + " must not be empty"));
    }

    std::uint64_t parsed = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto [ptr, error] = std::from_chars(begin, end, parsed);
    if (error != std::errc{} || ptr != end || parsed > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error(ConfigError(path, line_number, field_name + " must be an unsigned 32-bit integer"));
    }
    if (parsed == 0) {
        throw std::runtime_error(ConfigError(path, line_number, field_name + " must be greater than zero"));
    }

    return static_cast<std::uint32_t>(parsed);
}

bool ParseBoolField(const std::string& path, std::size_t line_number, std::string_view raw_value, const std::string& field_name) {
    std::string value = std::string(Trim(raw_value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }

    throw std::runtime_error(ConfigError(path, line_number, field_name + " must be true, false, 1, or 0"));
}

}  // namespace

ServiceConfig ParseServiceConfig(const std::string& path) {
    ServiceConfig config;
    std::ifstream input(path);
    if (!input.is_open()) {
        return config;
    }

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;

        const std::string_view effective_line = Trim(StripInlineComment(line));
        if (effective_line.empty()) {
            continue;
        }
        const std::size_t equals = effective_line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = TrimScalar(effective_line.substr(0, equals));
        const std::string value = TrimScalar(effective_line.substr(equals + 1));
        if (key == "bind_address") {
            if (value.empty()) {
                throw std::runtime_error(ConfigError(path, line_number, "bind_address must not be empty"));
            }
            config.bind_address = value;
        } else if (key == "default_tick_rate_hz") {
            config.default_tick_rate_hz = ParseUint32Field(path, line_number, value, key);
        } else if (key == "json_logs") {
            config.json_logs = ParseBoolField(path, line_number, value, key);
        }
    }

    return config;
}

}  // namespace openrcc::common
