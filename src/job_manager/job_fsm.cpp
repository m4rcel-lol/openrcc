#include "job_manager/job_fsm.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stop_token>
#include <thread>

#include <spdlog/spdlog.h>

namespace openrcc::job_manager {
namespace {

constexpr std::uint32_t kDefaultTickRateHz = 30;

}  // namespace

std::string ToString(JobState state) {
    switch (state) {
        case JobState::PENDING:
            return "PENDING";
        case JobState::RUNNING:
            return "RUNNING";
        case JobState::DRAINING:
            return "DRAINING";
        case JobState::CLOSED:
            return "CLOSED";
        case JobState::FAULTED:
            return "FAULTED";
    }
    return "UNKNOWN";
}

JobFsm::JobFsm(std::string job_id) : job_id_(std::move(job_id)), state_(JobState::PENDING), transitions_(), mutex_() {}

void JobFsm::TransitionTo(JobState next) {
    std::lock_guard<std::mutex> lock(mutex_);
    const bool legal =
        (state_ == JobState::PENDING && next == JobState::RUNNING) ||
        (state_ == JobState::RUNNING && (next == JobState::DRAINING || next == JobState::FAULTED)) ||
        (state_ == JobState::DRAINING && next == JobState::CLOSED) ||
        (state_ == JobState::DRAINING && next == JobState::FAULTED) ||
        (state_ == JobState::PENDING && next == JobState::FAULTED);

    if (!legal) {
        throw std::logic_error("Invalid transition for job " + job_id_ + ": " + ToString(state_) + " -> " + ToString(next));
    }

    transitions_.push_back({ToString(state_), ToString(next)});
    state_ = next;
}

JobState JobFsm::State() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::vector<std::pair<std::string, std::string>> JobFsm::TransitionLog() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return transitions_;
}

JobManager::JobManager() : mutex_(), jobs_(), counter_(0) {}

JobManager::~JobManager() {
    while (true) {
        std::jthread worker;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& entry : jobs_) {
                ManagedJob& job = entry.second;
                if (!job.worker.joinable()) {
                    continue;
                }
                job.worker.request_stop();
                worker = std::move(job.worker);
                break;
            }
        }
        if (!worker.joinable()) {
            return;
        }
    }
}

std::string JobManager::OpenJob(std::uint32_t tick_rate_hz) {
    std::lock_guard<std::mutex> lock(mutex_);
    const std::uint32_t effective_tick_rate_hz = tick_rate_hz == 0 ? kDefaultTickRateHz : tick_rate_hz;

    std::ostringstream id_builder;
    id_builder << "job-" << std::setw(6) << std::setfill('0') << ++counter_;
    const std::string job_id = id_builder.str();

    auto [it, inserted] = jobs_.try_emplace(job_id, job_id, effective_tick_rate_hz);
    if (!inserted) {
        throw std::logic_error("Internal error: duplicate job ID detected despite monotonic counter increment: " + job_id);
    }
    ManagedJob& job = it->second;
    job.fsm.TransitionTo(JobState::RUNNING);

    // ARCH: One std::jthread per job isolates runaway jobs and allows cooperative cancellation via stop_token.
    job.worker = std::jthread([this, job_id](std::stop_token stop_token) {
        while (true) {
            {
                std::lock_guard<std::mutex> guard(mutex_);
                auto found = jobs_.find(job_id);
                if (found == jobs_.end()) {
                    return;
                }
                ManagedJob& managed_job = found->second;
                const JobState state = managed_job.fsm.State();
                if (state == JobState::DRAINING) {
                    managed_job.fsm.TransitionTo(JobState::CLOSED);
                    return;
                }
                if (state == JobState::CLOSED || state == JobState::FAULTED) {
                    return;
                }
                if (!managed_job.pending_script.empty()) {
                    managed_job.memory_bytes = managed_job.pending_script.size();
                    managed_job.pending_script.clear();
                }
            }
            if (stop_token.stop_requested()) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    spdlog::info("Opened job {}", job_id);
    return job_id;
}

bool JobManager::ExecuteScript(const std::string& job_id, const std::string& source) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = jobs_.find(job_id);
    if (found == jobs_.end()) {
        return false;
    }
    if (found->second.fsm.State() != JobState::RUNNING) {
        return false;
    }
    found->second.pending_script = source;
    return true;
}

bool JobManager::CloseJob(const std::string& job_id, bool force) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = jobs_.find(job_id);
    if (found == jobs_.end()) {
        return false;
    }

    const JobState state = found->second.fsm.State();
    if (state == JobState::CLOSED || state == JobState::FAULTED) {
        return true;
    }

    if (force) {
        // ARCH: Forced close maps to FAULTED when bypassing drain to make shutdown semantics explicit in metrics.
        found->second.fsm.TransitionTo(JobState::FAULTED);
        found->second.worker.request_stop();
        return true;
    }

    if (state == JobState::RUNNING) {
        found->second.fsm.TransitionTo(JobState::DRAINING);
        found->second.worker.request_stop();
        return true;
    }
    if (state == JobState::DRAINING) {
        return true;
    }

    return false;
}

JobState JobManager::GetJobState(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = jobs_.find(job_id);
    if (found == jobs_.end()) {
        throw std::out_of_range("Unknown job: " + job_id);
    }
    return found->second.fsm.State();
}

JobSnapshot JobManager::GetJobSnapshot(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = jobs_.find(job_id);
    if (found == jobs_.end()) {
        throw std::out_of_range("Unknown job: " + job_id);
    }

    const auto uptime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - found->second.started_at);
    return JobSnapshot{
        found->second.fsm.State(),
        static_cast<std::uint64_t>(uptime.count()),
        found->second.memory_bytes,
        found->second.tick_rate_hz,
    };
}

std::tuple<std::uint64_t, std::uint64_t, std::uint64_t> JobManager::AggregateCounts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::uint64_t running = 0;
    std::uint64_t faulted = 0;
    for (const auto& entry : jobs_) {
        const JobState state = entry.second.fsm.State();
        if (state == JobState::RUNNING) {
            ++running;
        }
        if (state == JobState::FAULTED) {
            ++faulted;
        }
    }
    return {jobs_.size(), running, faulted};
}

void JobManager::DrainAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& entry : jobs_) {
        const JobState state = entry.second.fsm.State();
        if (state == JobState::RUNNING) {
            entry.second.fsm.TransitionTo(JobState::DRAINING);
            entry.second.worker.request_stop();
        }
    }
}

}  // namespace openrcc::job_manager
