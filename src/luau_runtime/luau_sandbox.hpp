#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct lua_State;

namespace openrcc::luau_runtime {

/**
 * Luau runtime sandbox for a single job VM.
 */
class LuauSandbox {
public:
    /**
     * Construct sandbox for one job.
     *
     * @param job_id Identifier exposed to scripts.
     */
    explicit LuauSandbox(std::string job_id);

    /**
     * Destroy the sandbox and VM.
     */
    ~LuauSandbox();

    LuauSandbox(const LuauSandbox&) = delete;
    LuauSandbox& operator=(const LuauSandbox&) = delete;

    /**
     * Execute source in the sandboxed VM.
     *
     * @param source Luau source string.
     * @return True on success.
     */
    bool Execute(const std::string& source);

    /**
     * Configure per-tick instruction budget.
     *
     * @param budget Maximum instructions per tick.
     */
    void SetInstructionBudget(std::uint32_t budget);

    /**
     * Return blocked capability attempts for testing and observability.
     *
     * @return Captured blocked operation log lines.
     */
    std::vector<std::string> BlockedAttempts() const;

private:
    /**
     * Initialize VM and apply capability model.
     */
    void InitializeVm();

    /**
     * Install safe global rcc helper table.
     */
    void InstallRccApi();

    std::string job_id_;
    lua_State* state_;
    std::uint32_t instruction_budget_;
    std::vector<std::string> blocked_attempts_;
};

}  // namespace openrcc::luau_runtime
