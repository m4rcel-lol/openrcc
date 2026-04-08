#include "job_manager/job_fsm.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stop_token>
#include <thread>

#include <spdlog/spdlog.h>

namespace openrcc::job_manager {

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

std::string JobManager::OpenJob(std::uint32_t tick_rate_hz) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream id_builder;
    id_builder << "job-" << std::setw(6) << std::setfill('0') << ++counter_;
    const std::string job_id = id_builder.str();

    auto [it, inserted] = jobs_.try_emplace(job_id, job_id, tick_rate_hz);
    if (!inserted) {
        throw std::logic_error("Duplicate job ID generated: " + job_id);
    }
    ManagedJob& job = it->second;
    job.fsm.TransitionTo(JobState::RUNNING);

    // ARCH: One std::jthread per job isolates runaway jobs and allows cooperative cancellation via stop_token.
    job.worker = std::jthread([this, job_id](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            {
                std::lock_guard<std::mutex> guard(mutex_);
                auto found = jobs_.find(job_id);
                if (found == jobs_.end()) {
                    return;
                }
                if (found->second.fsm.State() == JobState::DRAINING) {
                    found->second.fsm.TransitionTo(JobState::CLOSED);
                    return;
                }
                if (!found->second.pending_script.empty()) {
                    found->second.pending_script.clear();
                }
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
