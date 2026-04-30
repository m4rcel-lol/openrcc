#include "soap/soap_bridge.hpp"

#include <cctype>
#include <charconv>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace openrcc::soap {
namespace {

bool IsNameChar(char ch) {
    const unsigned char value = static_cast<unsigned char>(ch);
    return std::isalnum(value) != 0 || ch == '_' || ch == '-' || ch == ':' || ch == '.';
}

bool MatchesTagName(std::string_view candidate, std::string_view tag) {
    if (candidate == tag) {
        return true;
    }

    const std::size_t colon = candidate.rfind(':');
    return colon != std::string_view::npos && candidate.substr(colon + 1) == tag;
}

std::string_view Trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return value;
}

std::optional<std::uint32_t> ParseUint32(std::string_view value) {
    value = Trim(value);
    if (value.empty()) {
        return std::nullopt;
    }

    std::uint64_t parsed = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto [ptr, error] = std::from_chars(begin, end, parsed);
    if (error != std::errc{} || ptr != end || parsed > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }

    return static_cast<std::uint32_t>(parsed);
}

}  // namespace

std::string ExtractTagValue(const std::string& xml, const std::string& tag) {
    std::size_t cursor = 0;
    while (cursor < xml.size()) {
        const std::size_t open = xml.find('<', cursor);
        if (open == std::string::npos || open + 1 >= xml.size()) {
            return "";
        }

        const char first = xml[open + 1];
        if (first == '/' || first == '!' || first == '?') {
            cursor = open + 1;
            continue;
        }

        const std::size_t name_begin = open + 1;
        std::size_t name_end = name_begin;
        while (name_end < xml.size() && IsNameChar(xml[name_end])) {
            ++name_end;
        }
        if (name_end == name_begin) {
            cursor = open + 1;
            continue;
        }

        const std::string_view full_name(xml.data() + name_begin, name_end - name_begin);
        const std::size_t open_end = xml.find('>', name_end);
        if (open_end == std::string::npos) {
            return "";
        }

        if (!MatchesTagName(full_name, tag)) {
            cursor = open_end + 1;
            continue;
        }

        const std::string close = "</" + std::string(full_name) + ">";
        const std::size_t value_begin = open_end + 1;
        const std::size_t close_begin = xml.find(close, value_begin);
        if (close_begin == std::string::npos) {
            return "";
        }

        return xml.substr(value_begin, close_begin - value_begin);
    }

    return "";
}

openrcc::v1::OpenJobRequest MapSoapToOpenJob(const std::string& xml) {
    openrcc::v1::OpenJobRequest request;
    request.set_request_id(ExtractTagValue(xml, "request_id"));
    request.set_job_name(ExtractTagValue(xml, "job_name"));

    const std::string tick_rate = ExtractTagValue(xml, "tick_rate_hz");
    if (const std::optional<std::uint32_t> parsed = ParseUint32(tick_rate); parsed.has_value()) {
        request.set_tick_rate_hz(*parsed);
    }

    return request;
}

}  // namespace openrcc::soap
