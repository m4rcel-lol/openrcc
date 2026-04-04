#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace openrcc::job_manager {

enum class JobState {
    PENDING,
    RUNNING,
    DRAINING,
    CLOSED,
    FAULTED,
};

/**
 * Converts state enum to stable string for logging and APIs.
 *
 * @param state Current state.
 * @return Stable uppercase state name.
 */
std::string ToString(JobState state);

/**
 * Single-job finite state machine with guarded transitions.
 */
class JobFsm {
public:
    /**
     * Construct a job FSM.
     *
     * @param job_id Unique job identifier.
     */
    explicit JobFsm(std::string job_id);

    /**
     * Transition to a new state, enforcing legal edges.
     *
     * @param next Target state.
     * @throws std::logic_error on invalid transition.
     */
    void TransitionTo(JobState next);

    /**
     * Get current state.
     *
     * @return Current FSM state.
     */
    JobState State() const;

    /**
     * Get transition history as tuples of from/to state names.
     *
     * @return Ordered transition log.
     */
    std::vector<std::pair<std::string, std::string>> TransitionLog() const;

private:
    std::string job_id_;
    JobState state_;
    std::vector<std::pair<std::string, std::string>> transitions_;
    mutable std::mutex mutex_;
};

/**
 * Thread-owning job manager coordinating lifecycle for all jobs.
 */
class JobManager {
public:
    /**
     * Create a new manager.
     */
    JobManager();

    /**
     * Open a new job and start its worker thread.
     *
     * @param tick_rate_hz Tick rate assigned to the job.
     * @return Assigned job identifier.
     */
    std::string OpenJob(std::uint32_t tick_rate_hz);

    /**
     * Queue script execution against a job.
     *
     * @param job_id Target job identifier.
     * @param source Luau source string.
     * @return True if accepted.
     */
    bool ExecuteScript(const std::string& job_id, const std::string& source);

    /**
     * Close a tracked job.
     *
     * @param job_id Target job identifier.
     * @param force True to force immediate close.
     * @return True if close request was accepted.
     */
    bool CloseJob(const std::string& job_id, bool force);

    /**
     * Query state for a job.
     *
     * @param job_id Target job identifier.
     * @return Current state.
     */
    JobState GetJobState(const std::string& job_id) const;

    /**
     * Get aggregate statistics.
     *
     * @return Tuple of total, running, and faulted job counts.
     */
    std::tuple<std::uint64_t, std::uint64_t, std::uint64_t> AggregateCounts() const;

    /**
     * Request draining on all running jobs.
     */
    void DrainAll();

private:
    struct ManagedJob {
        JobFsm fsm;
        std::jthread worker;
        std::chrono::steady_clock::time_point started_at;
        std::uint32_t tick_rate_hz;
        std::string pending_script;
        std::uint64_t memory_bytes;

        ManagedJob(std::string job_id, std::uint32_t tick)
            : fsm(std::move(job_id)),
              worker(),
              started_at(std::chrono::steady_clock::now()),
              tick_rate_hz(tick),
              pending_script(),
              memory_bytes(0) {}
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, ManagedJob> jobs_;
    std::uint64_t counter_;
};

}  // namespace openrcc::job_manager
