// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>

#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/experimental/services/orchestration/TimeSyncServiceExtensions.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
using namespace std::chrono_literals;
using namespace SilKit::Tests;

const std::chrono::nanoseconds expectedTime{10ms};

std::mutex mx;
int cvCounter{};
std::condition_variable cv;

TEST(ITest_AsyncSimTask, test_async_simtask_lockstep)
{
    // Goal: have a foreign/user thread run in lockstep with the SimulationStepHandler.
    // The completer thread invokes CompleteSimulationStep after being signaled from the SimulationStepHandler.
    // The sync participant's SimulationStepHandler will run as often as possible.
    // Async may not start a new SimTask before calling CompleteSimulationtask to complete the current one.


    SimTestHarness testHarness({"Sync", "Async"}, "silkit://localhost:0");

    std::atomic<std::chrono::nanoseconds> syncTimeNs{0ns};
    std::atomic<bool> done{false};
    std::atomic<int> numActiveSimtasks{0};
    std::atomic<int> numSyncSimtasks{0};

    auto* sync = testHarness.GetParticipant("Sync")->GetOrCreateTimeSyncService();
    auto* asyncParticipant = testHarness.GetParticipant("Async");
    auto* async = testHarness.GetParticipant("Async")->GetOrCreateTimeSyncService();

    sync->SetSimulationStepHandler([&numSyncSimtasks](auto, std::chrono::nanoseconds /*duration*/) {
        // run as fast as possible
        numSyncSimtasks++;
    }, 1ms);

    async->SetSimulationStepHandlerAsync([&](auto now, auto) {
        std::cout << "Async SimTask now=" << now.count() << " numActiveSimtasks=" << numActiveSimtasks
                  << " numSyncSimtasks=" << numSyncSimtasks << std::endl;


        syncTimeNs = now;
        numActiveSimtasks++;

        if (done)
        {
            return;
        }

        //wait until counter is even
        std::unique_lock<decltype(mx)> lock(mx);
        cv.wait(lock, [] { return cvCounter % 2 == 0; });

        // increment so that completer thread will be notified
        cvCounter++;

        if (now == expectedTime)
        {
            //Only allow time progress up to expectedTime
            std::cout << "Stopping simulation at expected time" << std::endl;
            asyncParticipant->GetOrCreateLifecycleService()->Stop("Stop Test");
            done = true;
        }

        cv.notify_one();
    }, 1ms);

    auto completer = std::thread{[&]() {
        while (!done && (syncTimeNs.load() < expectedTime))
        {
            std::unique_lock<decltype(mx)> lock(mx);
            //wait for odd counter
            cv.wait(lock, [] { return cvCounter % 2 == 1; });
            if (done)
            {
                return;
            }
            std::cout << "Completer numActiveSimtasks=" << numActiveSimtasks << std::endl;
            if (numActiveSimtasks != 1)
            {
                ASSERT_EQ(numActiveSimtasks, 0)
                    << "Only one SimTask should be active until CompleteSimulationStep is called";
                done = true;
            }
            numActiveSimtasks--;
            async->CompleteSimulationStep();

            // let simstep handler continue
            cvCounter++;
            cv.notify_one();
        }
    }};
    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach timeout"
                                     << " numActiveSimtasks=" << numActiveSimtasks
                                     << " numSyncSimtasks=" << numSyncSimtasks;
    done = true;
    cv.notify_all();
    if (completer.joinable())
    {
        completer.join();
    }
}

TEST(ITest_AsyncSimTask, test_async_simtask_nodeadlock)
{
    // The async participant uses the CompleteSimTask calls to request next simulation step.
    // The sync participant will be used to check the time progress

    SimTestHarness testHarness({"Sync", "Async"}, "silkit://localhost:0");

    auto syncTimeNs{0ns};

    auto* sync = testHarness.GetParticipant("Sync")->GetOrCreateTimeSyncService();
    auto* asyncParticipant = testHarness.GetParticipant("Async");
    auto* async = testHarness.GetParticipant("Async")->GetOrCreateTimeSyncService();

    sync->SetSimulationStepHandler([&syncTimeNs](auto now, std::chrono::nanoseconds /*duration*/) {
        std::cout << "Sync SimTask now=" << now.count() << std::endl;
        syncTimeNs = now;
    }, 1ms);

    async->SetSimulationStepHandlerAsync([&](auto now, auto) {
        std::cout << "Async SimTask now=" << now.count() << " expectedTime=" << expectedTime.count() << std::endl;
        if (now == expectedTime)
        {
            std::cout << "Stopping simulation at expected time" << std::endl;
            asyncParticipant->GetOrCreateLifecycleService()->Stop("Test");
        }
        async->CompleteSimulationStep();
    }, 1ms);

    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach timeout";

    auto isSame = expectedTime == syncTimeNs;
    auto isOffByOne = (syncTimeNs == (expectedTime + 1ms)) || (syncTimeNs == (expectedTime - 1ms));
    ASSERT_TRUE(isSame || isOffByOne) << "Simulation time should be at most off by one expectedTime "
                                      << " (due to NextSimTask handling in distributed participants): "
                                      << " expectedTime=" << expectedTime.count() << " syncTime=" << syncTimeNs.count();
}

TEST(ITest_AsyncSimTask, test_async_simtask_different_periods)
{
    // The async and sync participant use different time periods to validate the that a slower participant does
    // not execute its simtask too often.

    SimTestHarness testHarness({"Sync", "Async"}, "silkit://localhost:0");

    auto syncTime{0ns};
    auto asyncTime{0ns};
    const int periodFactor = 10;

    auto* sync = testHarness.GetParticipant("Sync")->GetOrCreateTimeSyncService();
    auto* asyncParticipant = testHarness.GetParticipant("Async");
    auto* async = testHarness.GetParticipant("Async")->GetOrCreateTimeSyncService();
    int countSync = 0;
    int countAsync = 0;

    sync->SetSimulationStepHandler([&syncTime, &countSync](auto now, std::chrono::nanoseconds /*duration*/) {
        syncTime = now;
        countSync++;
    }, 1ms);

    async->SetSimulationStepHandlerAsync([&](auto now, auto) {
        asyncTime = now;
        countAsync++;
        if (countAsync > periodFactor * 100000)
        {
            asyncParticipant->GetOrCreateLifecycleService()->Stop("Test");
        }
        async->CompleteSimulationStep();
    }, periodFactor * 1ms);
    // validate that they are called approximately equally often
    ASSERT_TRUE(std::abs(countAsync * periodFactor - countSync) < periodFactor);
}


TEST(ITest_AsyncSimTask, test_async_simtask_multiple_completion_calls)
{
    // Verify that multiple CompleteSimTask calls do not trigger malicious behaviour

    SimTestHarness testHarness({"Sync", "Async"}, "silkit://localhost:0");

    auto syncTime{0ns};
    auto asyncTime{0ns};
    const int periodFactor = 7;

    auto* sync = testHarness.GetParticipant("Sync")->GetOrCreateTimeSyncService();
    auto* asyncParticipant = testHarness.GetParticipant("Async");
    auto* async = testHarness.GetParticipant("Async")->GetOrCreateTimeSyncService();
    int countSync = 0;
    int countAsync = 0;

    sync->SetSimulationStepHandler([&syncTime, &countSync](auto now, std::chrono::nanoseconds /*duration*/) {
        ASSERT_TRUE(now - syncTime == 1ms);
        syncTime = now;
        countSync++;
    }, 1ms);

    async->SetSimulationStepHandlerAsync([&](auto now, auto) {
        ASSERT_TRUE(now - asyncTime == periodFactor * 1ms);
        asyncTime = now;
        countAsync++;
        if (countAsync > periodFactor * 100000)
        {
            asyncParticipant->GetOrCreateLifecycleService()->Stop("Test");
        }
        async->CompleteSimulationStep();
        async->CompleteSimulationStep();
        async->CompleteSimulationStep();
    }, periodFactor * 1ms);
    // validate that they are called approximately equally often
    ASSERT_TRUE(std::abs(countAsync * periodFactor - countSync) < periodFactor);
}

struct ParticipantData
{
    std::mutex mx;
    std::condition_variable cv;
    bool needsComplete{false};
    std::atomic<bool> running{true};
    std::vector<std::chrono::steady_clock::time_point> stepStarted;
    std::vector<std::chrono::steady_clock::time_point> stepCompleted;

    void AssertValid() const
    {
        ASSERT_FALSE(running);
        ASSERT_FALSE(needsComplete);

        for (size_t i = 0; i < std::min(stepStarted.size(), stepCompleted.size()); ++i)
        {
            ASSERT_LT(stepStarted[i], stepCompleted[i]);
            if (i + 1 < stepStarted.size())
            {
                ASSERT_LT(stepCompleted[i], stepStarted[i + 1]);
            }
        }
    }

    void AssertTimestampsAlwaysOutsideStep(const std::vector<std::chrono::steady_clock::time_point>& timestamps) const
    {
        for (size_t i = 0; i < std::min(stepStarted.size(), stepCompleted.size()); ++i)
        {
            for (const auto& timestamp : timestamps)
            {
                ASSERT_FALSE((stepStarted[i] < timestamp) && (timestamp < stepCompleted[i]));
            }
        }
    }
};

auto MakeNormalSimulationStepHandlerAsync(ParticipantData* d)
    -> SilKit::Services::Orchestration::ITimeSyncService::SimulationStepHandler
{
    return SilKit::Services::Orchestration::ITimeSyncService::SimulationStepHandler{[d](auto, auto) {
        d->stepStarted.emplace_back(std::chrono::steady_clock::now());

        {
            std::lock_guard<std::mutex> lock(d->mx);
            d->needsComplete = true;
        }

        d->cv.notify_one();
    }};
}

auto MakeCompletionThread(SimParticipant* p, ParticipantData* d) -> std::thread
{
    return std::thread{[p, d] {
        while (d->running)
        {
            std::unique_lock<std::mutex> lock(d->mx);

            d->cv.wait_for(lock, 10ms, [d] { return d->needsComplete; });

            if (d->needsComplete)
            {
                d->needsComplete = false;
                d->stepCompleted.emplace_back(std::chrono::steady_clock::now());
                p->GetOrCreateTimeSyncService()->CompleteSimulationStep();
            }
        }
    }};
}

TEST(ITest_AsyncSimTask, test_async_simtask_other_simulation_steps_completed_handler)
{
    SimTestHarness testHarness({"A", "B", "C"}, "silkit://localhost:0");

    const auto a = testHarness.GetParticipant("A");
    const auto b = testHarness.GetParticipant("B");
    const auto c = testHarness.GetParticipant("C");

    ParticipantData ad, bd, cd;

    std::vector<std::chrono::steady_clock::time_point> stepLastOpen;

    a->GetOrCreateLifecycleService()->SetStopHandler([&ad] { ad.running = false; });
    b->GetOrCreateLifecycleService()->SetStopHandler([&bd] { bd.running = false; });
    c->GetOrCreateLifecycleService()->SetStopHandler([&cd] { cd.running = false; });

    const auto aLifecycleService = a->GetOrCreateLifecycleService();

    a->GetOrCreateTimeSyncService()->SetSimulationStepHandlerAsync([aLifecycleService, &ad](auto now, auto) {
        ad.stepStarted.emplace_back(std::chrono::steady_clock::now());

        if (now > 90ms)
        {
            aLifecycleService->Stop("STOP");
        }
    }, 1ms);

    SilKit::Experimental::Services::Orchestration::AddOtherSimulationStepsCompletedHandler(
        a->GetOrCreateTimeSyncService(), [&ad, &stepLastOpen] {
        stepLastOpen.emplace_back(std::chrono::steady_clock::now());

        {
            std::unique_lock<std::mutex> lock{ad.mx};
            ad.needsComplete = true;
        }

        ad.cv.notify_one();
    });

    b->GetOrCreateTimeSyncService()->SetSimulationStepHandlerAsync(MakeNormalSimulationStepHandlerAsync(&bd), 3ms);
    c->GetOrCreateTimeSyncService()->SetSimulationStepHandlerAsync(MakeNormalSimulationStepHandlerAsync(&cd), 10ms);

    auto aCompletionThread = MakeCompletionThread(a, &ad);
    auto bCompletionThread = MakeCompletionThread(b, &bd);
    auto cCompletionThread = MakeCompletionThread(c, &cd);

    const bool stopped = testHarness.Run(10s);
    ASSERT_TRUE(stopped);

    aCompletionThread.join();
    bCompletionThread.join();
    cCompletionThread.join();

    ad.AssertValid();

    bd.AssertValid();
    bd.AssertTimestampsAlwaysOutsideStep(stepLastOpen);

    cd.AssertValid();
    cd.AssertTimestampsAlwaysOutsideStep(stepLastOpen);
}

} // anonymous namespace
