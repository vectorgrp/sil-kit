// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "services/orchestration/TimeCoordinator.hpp"

namespace {

using namespace std::chrono_literals;
using namespace testing;
using namespace SilKit::Services::Orchestration;

auto Task(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration) -> NextSimTask
{
    NextSimTask task;
    task.timePoint = timePoint;
    task.duration = duration;
    return task;
}

TEST(Test_TimeCoordinator, no_grant_until_all_participants_reported)
{
    TimeCoordinator coordinator;
    EXPECT_FALSE(coordinator.AddParticipant("A"));
    EXPECT_FALSE(coordinator.AddParticipant("B"));

    EXPECT_FALSE(coordinator.UpdateNextSimTask("A", Task(0ms, 1ms)));
    EXPECT_FALSE(coordinator.CurrentGrant());

    auto dispatch = coordinator.UpdateNextSimTask("B", Task(0ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 0ms);
    EXPECT_THAT(dispatch->recipients, UnorderedElementsAre("A", "B"));
}

TEST(Test_TimeCoordinator, lockstep_grants_once_per_step)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    coordinator.AddParticipant("B");
    coordinator.AddParticipant("C");
    coordinator.UpdateNextSimTask("A", Task(0ms, 1ms));
    coordinator.UpdateNextSimTask("B", Task(0ms, 1ms));
    ASSERT_TRUE(coordinator.UpdateNextSimTask("C", Task(0ms, 1ms)));

    // The first two reports of the next step do not change T_min
    EXPECT_FALSE(coordinator.UpdateNextSimTask("A", Task(1ms, 1ms)));
    EXPECT_FALSE(coordinator.UpdateNextSimTask("B", Task(1ms, 1ms)));

    auto dispatch = coordinator.UpdateNextSimTask("C", Task(1ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 1ms);
    EXPECT_THAT(dispatch->recipients, UnorderedElementsAre("A", "B", "C"));
}

TEST(Test_TimeCoordinator, heterogeneous_step_sizes_grant_only_eligible_participants)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("Fast");
    coordinator.AddParticipant("Slow");
    coordinator.UpdateNextSimTask("Fast", Task(0ms, 1ms));
    ASSERT_TRUE(coordinator.UpdateNextSimTask("Slow", Task(0ms, 3ms)));

    // Slow steps to 3ms. Fast is the only one eligible until it catches up.
    EXPECT_FALSE(coordinator.UpdateNextSimTask("Slow", Task(3ms, 3ms)));
    for (auto t = 1ms; t < 3ms; t += 1ms)
    {
        auto dispatch = coordinator.UpdateNextSimTask("Fast", Task(t, 1ms));
        ASSERT_TRUE(dispatch);
        EXPECT_EQ(dispatch->grant.timePoint, t);
        EXPECT_THAT(dispatch->recipients, ElementsAre("Fast"));
    }

    auto dispatch = coordinator.UpdateNextSimTask("Fast", Task(3ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 3ms);
    EXPECT_THAT(dispatch->recipients, UnorderedElementsAre("Fast", "Slow"));
}

TEST(Test_TimeCoordinator, grant_duration_is_earliest_boundary_after_grant)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    coordinator.AddParticipant("B");
    coordinator.UpdateNextSimTask("A", Task(0ms, 10ms));
    auto dispatch = coordinator.UpdateNextSimTask("B", Task(0ms, 3ms));
    ASSERT_TRUE(dispatch);
    // End of B's step is the earliest boundary
    EXPECT_EQ(dispatch->grant.duration, 3ms);

    // A advances to 10ms, B to 3ms: B is eligible, the next boundary is B's end (5ms)
    coordinator.UpdateNextSimTask("A", Task(10ms, 10ms));
    dispatch = coordinator.UpdateNextSimTask("B", Task(3ms, 2ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 3ms);
    EXPECT_EQ(dispatch->grant.duration, 2ms);
}

TEST(Test_TimeCoordinator, removal_unblocks)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    coordinator.AddParticipant("B");
    EXPECT_FALSE(coordinator.UpdateNextSimTask("A", Task(0ms, 1ms)));

    auto dispatch = coordinator.RemoveParticipant("B");
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 0ms);
    EXPECT_THAT(dispatch->recipients, ElementsAre("A"));

    EXPECT_FALSE(coordinator.RemoveParticipant("Unknown"));
}

TEST(Test_TimeCoordinator, unknown_participant_is_ignored)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    EXPECT_FALSE(coordinator.UpdateNextSimTask("Unknown", Task(0ms, 1ms)));
    EXPECT_FALSE(coordinator.HasParticipant("Unknown"));
}

TEST(Test_TimeCoordinator, late_joiner_is_clamped_to_current_time)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    coordinator.UpdateNextSimTask("A", Task(0ms, 1ms));
    for (auto t = 1ms; t <= 5ms; t += 1ms)
    {
        ASSERT_TRUE(coordinator.UpdateNextSimTask("A", Task(t, 1ms)));
    }

    // The joiner gets the current grant on discovery, it does not move the time back
    auto dispatch = coordinator.AddParticipant("Joiner");
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 5ms);
    EXPECT_THAT(dispatch->recipients, ElementsAre("Joiner"));

    // A completes its step but must wait for the joiner
    EXPECT_FALSE(coordinator.UpdateNextSimTask("A", Task(6ms, 1ms)));

    // The joiner reports its initial time: it is still eligible at the current grant (hop-on)
    dispatch = coordinator.UpdateNextSimTask("Joiner", Task(0ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 5ms);
    EXPECT_THAT(dispatch->recipients, ElementsAre("Joiner"));

    // After its hop-on step, the time advances for both
    dispatch = coordinator.UpdateNextSimTask("Joiner", Task(6ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 6ms);
    EXPECT_THAT(dispatch->recipients, UnorderedElementsAre("A", "Joiner"));
}

TEST(Test_TimeCoordinator, duplicate_report_resends_current_grant)
{
    TimeCoordinator coordinator;
    coordinator.AddParticipant("A");
    coordinator.AddParticipant("B");
    coordinator.UpdateNextSimTask("A", Task(0ms, 1ms));
    ASSERT_TRUE(coordinator.UpdateNextSimTask("B", Task(0ms, 1ms)));

    auto dispatch = coordinator.UpdateNextSimTask("B", Task(0ms, 1ms));
    ASSERT_TRUE(dispatch);
    EXPECT_EQ(dispatch->grant.timePoint, 0ms);
    EXPECT_THAT(dispatch->recipients, ElementsAre("B"));
}

} // namespace
