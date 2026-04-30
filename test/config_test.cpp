#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "common/config.hpp"

namespace {

std::filesystem::path WriteTempConfig(const std::string& contents) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("openrcc-config-test-" + std::to_string(now) + ".toml");

    std::ofstream output(path);
    output << contents;
    output.close();
    return path;
}

}  // namespace

TEST(ConfigTest, MissingFileUsesDefaults) {
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "openrcc-config-test-file-that-does-not-exist.toml";

    const openrcc::common::ServiceConfig config = openrcc::common::ParseServiceConfig(missing.string());

    EXPECT_EQ(config.bind_address, "0.0.0.0:50051");
    EXPECT_EQ(config.default_tick_rate_hz, 30U);
    EXPECT_TRUE(config.json_logs);
}

TEST(ConfigTest, ParsesSupportedFieldsAndInlineComments) {
    const std::filesystem::path path = WriteTempConfig(R"(
bind_address = "127.0.0.1:60000" # local development
default_tick_rate_hz = 60
json_logs = false
)");

    const openrcc::common::ServiceConfig config = openrcc::common::ParseServiceConfig(path.string());
    std::filesystem::remove(path);

    EXPECT_EQ(config.bind_address, "127.0.0.1:60000");
    EXPECT_EQ(config.default_tick_rate_hz, 60U);
    EXPECT_FALSE(config.json_logs);
}

TEST(ConfigTest, RejectsInvalidTickRate) {
    const std::filesystem::path path = WriteTempConfig("default_tick_rate_hz = 0\n");

    EXPECT_THROW((void)openrcc::common::ParseServiceConfig(path.string()), std::runtime_error);
    std::filesystem::remove(path);
}

TEST(ConfigTest, RejectsInvalidBool) {
    const std::filesystem::path path = WriteTempConfig("json_logs = sometimes\n");

    EXPECT_THROW((void)openrcc::common::ParseServiceConfig(path.string()), std::runtime_error);
    std::filesystem::remove(path);
}
