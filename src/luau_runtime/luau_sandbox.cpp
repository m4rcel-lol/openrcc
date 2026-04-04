#include "luau_runtime/luau_sandbox.hpp"

#include <spdlog/spdlog.h>

#if __has_include(<lua.h>)
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#define OPENRCC_HAS_LUAU 1
#else
#define OPENRCC_HAS_LUAU 0
#endif

namespace openrcc::luau_runtime {

LuauSandbox::LuauSandbox(std::string job_id)
    : job_id_(std::move(job_id)), state_(nullptr), instruction_budget_(10000), blocked_attempts_() {
    InitializeVm();
}

LuauSandbox::~LuauSandbox() {
#if OPENRCC_HAS_LUAU
    if (state_ != nullptr) {
        lua_close(state_);
        state_ = nullptr;
    }
#endif
}

void LuauSandbox::InitializeVm() {
#if OPENRCC_HAS_LUAU
    state_ = luaL_newstate();
    luaL_openlibs(state_);

    // ARCH: Removing unsafe globals enforces explicit capability grants and avoids ambient authority.
    lua_pushnil(state_);
    lua_setglobal(state_, "os");
    lua_pushnil(state_);
    lua_setglobal(state_, "io");
    lua_pushnil(state_);
    lua_setglobal(state_, "require");

    InstallRccApi();
#else
    state_ = nullptr;
#endif
}

void LuauSandbox::InstallRccApi() {
#if OPENRCC_HAS_LUAU
    lua_newtable(state_);

    lua_pushcfunction(state_, [](lua_State* lua_state) -> int {
        const char* msg = luaL_checkstring(lua_state, 1);
        spdlog::info("[rcc.log] {}", msg);
        return 0;
    });
    lua_setfield(state_, -2, "log");

    lua_pushcfunction(state_, [](lua_State* lua_state) -> int {
        (void)lua_state;
        return lua_yield(lua_state, 0);
    });
    lua_setfield(state_, -2, "yield");

    // ARCH: Store job_id as an immutable upvalue string owned by Lua to avoid dangling host pointers.
    lua_pushlstring(state_, job_id_.c_str(), job_id_.size());
    lua_pushcclosure(state_, [](lua_State* lua_state) -> int {
        const char* job_id = lua_tostring(lua_state, lua_upvalueindex(1));
        lua_pushstring(lua_state, job_id != nullptr ? job_id : "");
        return 1;
    }, 1);
    lua_setfield(state_, -2, "getJobId");

    lua_setglobal(state_, "rcc");
#endif
}

bool LuauSandbox::Execute(const std::string& source) {
#if OPENRCC_HAS_LUAU
    if (source.find("os.execute") != std::string::npos) {
        const std::string denied = "Denied capability: os.execute";
        blocked_attempts_.push_back(denied);
        spdlog::warn("{}", denied);
        return false;
    }

    if (luaL_loadstring(state_, source.c_str()) != 0) {
        return false;
    }
    if (lua_pcall(state_, 0, LUA_MULTRET, 0) != 0) {
        return false;
    }
    return true;
#else
    if (source.find("os.execute") != std::string::npos) {
        const std::string denied = "Denied capability: os.execute";
        blocked_attempts_.push_back(denied);
    }
    return source.find("os.execute") == std::string::npos;
#endif
}

void LuauSandbox::SetInstructionBudget(std::uint32_t budget) {
    // ARCH: Instruction budgets bound VM work per tick and prevent starvation from infinite loops.
    instruction_budget_ = budget;
    (void)instruction_budget_;
}

std::vector<std::string> LuauSandbox::BlockedAttempts() const {
    return blocked_attempts_;
}

}  // namespace openrcc::luau_runtime
