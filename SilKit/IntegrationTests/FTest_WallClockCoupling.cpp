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

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
using namespace std::chrono_literals;
using namespace SilKit::Tests;


TEST(FTest_WallClockCoupling, test_wallclock_sync_simtask)
{
    double animationFactor = 2.0;
    std::string configWithAnimFactor = R"({"Experimental": {"TimeSynchronization": { "AnimationFactor": 2.0 } }})";

    SimTestHarness testHarness({"P1", "P2", "P3"}, "silkit://localhost:0", true);

    auto wc_P1{0ns};
    auto wc_P2{0ns};
    auto wc_P3{0ns};

    auto* P1 = testHarness.GetParticipant("P1", configWithAnimFactor);
    auto* lifeCycleService_P1 = P1->GetOrCreateLifecycleService();
    auto* timeSyncService_P1 = P1->GetOrCreateTimeSyncService();

    auto* timeSyncService_P2 = testHarness.GetParticipant("P2")->GetOrCreateTimeSyncService();
    auto* timeSyncService_P3 = testHarness.GetParticipant("P3")->GetOrCreateTimeSyncService();

    auto stepSize = 50ms;
    std::chrono::nanoseconds simDuration = 2s;

    timeSyncService_P1->SetSimulationStepHandler(
        [simDuration, &wc_P1, &lifeCycleService_P1](std::chrono::nanoseconds now,
                                                    std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P1 = std::chrono::system_clock::now();
        if (now <= simDuration)
        {
            wc_P1 = std::chrono::system_clock::now() - timeSimStart_P1;
        }

        if (now == simDuration)
            lifeCycleService_P1->Stop("Stopping Test");
    },
        stepSize);

    timeSyncService_P2->SetSimulationStepHandler(
        [simDuration, &wc_P2](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P2 = std::chrono::system_clock::now();
        if (now <= simDuration)
            wc_P2 = std::chrono::system_clock::now() - timeSimStart_P2;
    }, stepSize);
    timeSyncService_P3->SetSimulationStepHandler(
        [simDuration, &wc_P3](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P3 = std::chrono::system_clock::now();
        if (now <= simDuration)
            wc_P3 = std::chrono::system_clock::now() - timeSimStart_P3;
    }, stepSize);

    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach timeout";

    std::chrono::nanoseconds acceptedDeviation = 20ms;
    auto evalWallClockDeviation = [animationFactor, acceptedDeviation,
                                   simDuration](std::chrono::nanoseconds wallClock) {
        auto wallClockDeviationFromVirtualTime = std::abs(wallClock.count() / animationFactor - simDuration.count());
        std::cout << "Wall clock deviation from virtual time: " << wallClockDeviationFromVirtualTime / 1e6 << "ms"
                  << std::endl;
        EXPECT_LT(wallClockDeviationFromVirtualTime, acceptedDeviation.count());
    };

    evalWallClockDeviation(wc_P1);
    evalWallClockDeviation(wc_P2);
    evalWallClockDeviation(wc_P3);
}


std::mutex mx;
bool doStep = false;
std::condition_variable cv;

TEST(FTest_WallClockCoupling, test_wallclock_mixed_simtask)
{
    double animationFactor = 2.0;
    std::string configWithAnimFactor = R"({"Experimental": {"TimeSynchronization": { "AnimationFactor": 2.0 } }})";

    SimTestHarness testHarness({"P1", "P2", "P3"}, "silkit://localhost:0", true);

    auto wc_P1{0ns};
    auto wc_P2{0ns};
    auto wc_P3{0ns};

    auto* P1 = testHarness.GetParticipant("P1", configWithAnimFactor);
    auto* lifeCycleService_P1 = P1->GetOrCreateLifecycleService();
    auto* timeSyncService_P1 = P1->GetOrCreateTimeSyncService();

    auto* timeSyncService_P2 = testHarness.GetParticipant("P2")->GetOrCreateTimeSyncService();
    auto* timeSyncService_P3 = testHarness.GetParticipant("P3")->GetOrCreateTimeSyncService();

    auto stepSize = 50ms;
    std::chrono::nanoseconds simDuration = 2s;

    std::atomic<bool> done{false};
    timeSyncService_P1->SetSimulationStepHandlerAsync(
        [&done, simDuration, &wc_P1, &lifeCycleService_P1](std::chrono::nanoseconds now,
                                                           std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P1 = std::chrono::system_clock::now();
        if (now <= simDuration)
        {
            wc_P1 = std::chrono::system_clock::now() - timeSimStart_P1;
        }

        if (now == simDuration)
        {
            lifeCycleService_P1->Stop("Stopping Test");
            done = true;
        }

        std::unique_lock<decltype(mx)> lock(mx);
        doStep = true;
        cv.notify_one();

        if (done)
        {
            return;
        }
    },
        stepSize);

    timeSyncService_P2->SetSimulationStepHandler(
        [simDuration, &wc_P2](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P2 = std::chrono::system_clock::now();
        if (now <= simDuration)
            wc_P2 = std::chrono::system_clock::now() - timeSimStart_P2;
    }, stepSize);
    timeSyncService_P3->SetSimulationStepHandler(
        [simDuration, &wc_P3](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
        static auto timeSimStart_P3 = std::chrono::system_clock::now();
        if (now <= simDuration)
            wc_P3 = std::chrono::system_clock::now() - timeSimStart_P3;
    }, stepSize);


    auto completer = std::thread{[&done, timeSyncService_P1]() {
        while (!done)
        {
            std::unique_lock<decltype(mx)> lock(mx);
            cv.wait(lock, [] { return doStep; });
            doStep = false;

            timeSyncService_P1->CompleteSimulationStep();

            if (done)
            {
                return;
            }
        }
    }};

    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach timeout";

    std::chrono::nanoseconds acceptedDeviation = 20ms;
    auto evalWallClockDeviation = [animationFactor, acceptedDeviation,
                                   simDuration](std::chrono::nanoseconds wallClock) {
        auto wallClockDeviationFromVirtualTime = std::abs(wallClock.count() / animationFactor - simDuration.count());
        std::cout << "Wall clock deviation from virtual time: " << wallClockDeviationFromVirtualTime / 1e6 << "ms"
                  << std::endl;
        EXPECT_LT(wallClockDeviationFromVirtualTime, acceptedDeviation.count());
    };

    evalWallClockDeviation(wc_P1);
    evalWallClockDeviation(wc_P2);
    evalWallClockDeviation(wc_P3);

    done = true;
    cv.notify_one();
    if (completer.joinable())
    {
        completer.join();
    }
}


} // anonymous namespace
