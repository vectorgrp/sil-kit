// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "silkit/services/orchestration/all.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// These tests only check lower bounds of the elapsed wall clock time. A slow or heavily loaded machine (e.g., a CI
// runner) can only make a simulation run longer, never shorter, so lower bounds are robust. Upper bounds are only
// enforced by the generous harness timeout. For precise timing measurements, see FTest_WallClockCoupling.

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Tests;

using Clock = std::chrono::steady_clock;

const std::string configWithAnimationFactorOne{
    R"({"Experimental": {"TimeSynchronization": {"AnimationFactor": 1.0}}})"};
const std::string configWithAnimationFactorTwo{
    R"({"Experimental": {"TimeSynchronization": {"AnimationFactor": 2.0}}})"};

constexpr auto harnessTimeout = 30s;

// The first simulation step is executed right away, so the coupled run lasts one step less than the simulated duration.
auto MinimumWallClockDuration(std::chrono::nanoseconds simDuration, std::chrono::nanoseconds stepSize,
                              double animationFactor) -> std::chrono::nanoseconds
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>((simDuration - stepSize) * animationFactor);
}

// For readable failure messages
template <typename Duration>
auto ToMs(Duration duration) -> double
{
    return std::chrono::duration<double, std::milli>(duration).count();
}

// A participant coupled to the wall clock slows down the whole simulation, including participants without
// wall clock coupling.
TEST(ITest_WallClockCoupling, coupled_participant_paces_the_whole_simulation)
{
    const auto stepSize = 10ms;
    const std::chrono::nanoseconds simDuration = 200ms;
    const double animationFactor = 2.0;

    SimTestHarnessArgs args;
    args.syncParticipantNames = {"Coupled", "Uncoupled"};
    args.deferParticipantCreation = true;
    SimTestHarness testHarness{args};

    auto* coupled = testHarness.GetParticipant("Coupled", configWithAnimationFactorTwo);
    auto* coupledLifecycleService = coupled->GetOrCreateLifecycleService();
    auto* uncoupled = testHarness.GetParticipant("Uncoupled");

    Clock::time_point coupledStart{};
    Clock::time_point coupledEnd{};
    Clock::time_point uncoupledStart{};
    Clock::time_point uncoupledEnd{};

    coupled->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (now == 0ns)
        {
            coupledStart = Clock::now();
        }
        if (now == simDuration)
        {
            coupledEnd = Clock::now();
            coupledLifecycleService->Stop("Simulated duration reached");
        }
    }, stepSize);

    // The uncoupled participant has no wall clock coupling on its own, but it must wait for the coupled one
    uncoupled->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (now == 0ns)
        {
            uncoupledStart = Clock::now();
        }
        if (now == simDuration)
        {
            uncoupledEnd = Clock::now();
        }
    }, stepSize);

    ASSERT_TRUE(testHarness.Run(harnessTimeout)) << "SimTestHarness should terminate without timeout";

    const auto minimumDuration = MinimumWallClockDuration(simDuration, stepSize, animationFactor);
    EXPECT_GE(ToMs(coupledEnd - coupledStart), ToMs(minimumDuration));
    EXPECT_GE(ToMs(uncoupledEnd - uncoupledStart), ToMs(minimumDuration));
}

// If async simulation steps complete later than the wall clock requires, the simulation catches up after each
// completion without losing or repeating steps.
TEST(ITest_WallClockCoupling, lagging_async_step_completion_catches_up)
{
    const auto stepSize = 10ms;
    const std::chrono::nanoseconds simDuration = 100ms;
    const auto completionDelay = 25ms;
    const auto numSteps = static_cast<int>(simDuration / stepSize) + 1; // including the step at 0ns

    SimTestHarnessArgs args;
    args.syncParticipantNames = {"Coupled"};
    args.deferParticipantCreation = true;
    SimTestHarness testHarness{args};

    auto* coupled = testHarness.GetParticipant("Coupled", configWithAnimationFactorOne);
    auto* lifecycleService = coupled->GetOrCreateLifecycleService();
    auto* timeSyncService = coupled->GetOrCreateTimeSyncService();

    std::mutex mutex;
    std::condition_variable cv;
    int pendingCompletions = 0;
    bool done = false;

    std::vector<std::chrono::nanoseconds> stepTimes;
    Clock::time_point start{};
    Clock::time_point end{};

    timeSyncService->SetSimulationStepHandlerAsync([&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (now == 0ns)
        {
            start = Clock::now();
        }
        stepTimes.push_back(now);
        if (now == simDuration)
        {
            end = Clock::now();
            lifecycleService->Stop("Simulated duration reached");
        }

        std::lock_guard<std::mutex> lock{mutex};
        ++pendingCompletions;
        cv.notify_one();
    }, stepSize);

    std::thread completer{[&] {
        std::unique_lock<std::mutex> lock{mutex};
        while (true)
        {
            cv.wait(lock, [&] { return pendingCompletions > 0 || done; });
            if (pendingCompletions == 0 && done)
            {
                return;
            }
            --pendingCompletions;

            lock.unlock();
            std::this_thread::sleep_for(completionDelay);
            timeSyncService->CompleteSimulationStep();
            lock.lock();
        }
    }};

    const auto ok = testHarness.Run(harnessTimeout);

    {
        std::lock_guard<std::mutex> lock{mutex};
        done = true;
        cv.notify_one();
    }
    completer.join();

    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    // No step was skipped or repeated
    std::vector<std::chrono::nanoseconds> expectedStepTimes;
    for (int i = 0; i < numSteps; ++i)
    {
        expectedStepTimes.push_back(i * stepSize);
    }
    EXPECT_EQ(stepTimes, expectedStepTimes);

    // The completion delay, not the wall clock, determines the pace
    EXPECT_GE(ToMs(end - start), ToMs((numSteps - 1) * completionDelay));
}

} // anonymous namespace
