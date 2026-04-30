#include <gtest/gtest.h>

#include "luau_runtime/luau_sandbox.hpp"

TEST(LuauSandboxTest, BlocksOsExecuteAndLogsAttempt) {
    openrcc::luau_runtime::LuauSandbox sandbox("job-test");
    const bool ok = sandbox.Execute("os.execute('echo unsafe')");
    EXPECT_FALSE(ok);

    const std::vector<std::string> blocked = sandbox.BlockedAttempts();
    ASSERT_FALSE(blocked.empty());
    EXPECT_NE(blocked.front().find("os.execute"), std::string::npos);
}

TEST(LuauSandboxTest, BlocksAmbientFileAndRequireCapabilities) {
    openrcc::luau_runtime::LuauSandbox sandbox("job-test");

    EXPECT_FALSE(sandbox.Execute("io.open('/tmp/openrcc-test', 'w')"));
    EXPECT_FALSE(sandbox.Execute("require('unsafe_module')"));

    const std::vector<std::string> blocked = sandbox.BlockedAttempts();
    ASSERT_EQ(blocked.size(), 2U);
    EXPECT_NE(blocked[0].find("io."), std::string::npos);
    EXPECT_NE(blocked[1].find("require("), std::string::npos);
}
