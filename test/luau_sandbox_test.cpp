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
