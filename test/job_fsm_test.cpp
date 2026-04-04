#include <gtest/gtest.h>

#include "job_manager/job_fsm.hpp"

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
