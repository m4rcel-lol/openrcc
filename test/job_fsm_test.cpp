#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#include "job_manager/job_fsm.hpp"

namespace {

bool WaitForState(openrcc::job_manager::JobManager& manager,
                  const std::string& job_id,
                  openrcc::job_manager::JobState expected) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        if (manager.GetJobState(job_id) == expected) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

}  // namespace

TEST(JobFsmTest, ValidTransitionsSucceed) {
    openrcc::job_manager::JobFsm fsm("job-test");
    EXPECT_EQ(fsm.State(), openrcc::job_manager::JobState::PENDING);

    fsm.TransitionTo(openrcc::job_manager::JobState::RUNNING);
    fsm.TransitionTo(openrcc::job_manager::JobState::DRAINING);
    fsm.TransitionTo(openrcc::job_manager::JobState::CLOSED);

    EXPECT_EQ(fsm.State(), openrcc::job_manager::JobState::CLOSED);
}

TEST(JobFsmTest, InvalidTransitionThrows) {
    openrcc::job_manager::JobFsm fsm("job-test");
    EXPECT_THROW(fsm.TransitionTo(openrcc::job_manager::JobState::CLOSED), std::logic_error);
}

TEST(JobFsmTest, ForceCanFaultDrainingJob) {
    openrcc::job_manager::JobFsm fsm("job-test");
    fsm.TransitionTo(openrcc::job_manager::JobState::RUNNING);
    fsm.TransitionTo(openrcc::job_manager::JobState::DRAINING);
    fsm.TransitionTo(openrcc::job_manager::JobState::FAULTED);

    EXPECT_EQ(fsm.State(), openrcc::job_manager::JobState::FAULTED);
}

TEST(JobManagerTest, GracefulCloseReachesClosed) {
    openrcc::job_manager::JobManager manager;
    const std::string job_id = manager.OpenJob(30);

    ASSERT_TRUE(manager.CloseJob(job_id, false));
    EXPECT_TRUE(WaitForState(manager, job_id, openrcc::job_manager::JobState::CLOSED));
}

TEST(JobManagerTest, OpenJobUsesFallbackTickRateWhenUnset) {
    openrcc::job_manager::JobManager manager;
    const std::string job_id = manager.OpenJob(0);

    const openrcc::job_manager::JobSnapshot snapshot = manager.GetJobSnapshot(job_id);
    EXPECT_EQ(snapshot.tick_rate_hz, 30U);
}
