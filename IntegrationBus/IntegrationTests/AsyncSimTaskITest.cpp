// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <atomic>

#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
using namespace std::chrono_literals;
using namespace ib::test;

const std::chrono::nanoseconds expectedTime{10ms};

TEST(AsyncSimTaskITest, test_async_simtask_nodeadlock)
{
    // The async participant uses the CompleteSimTask calls to request next simulation step.
    // The sync participant will be used to check the time progress

    SimTestHarness testHarness({"Sync", "Async"}, 42);

    auto syncTimeNs{0ns};

    auto* sync = testHarness.GetParticipant("Sync")->ComAdapter()->GetParticipantController();
    auto* asyncComAdapter = testHarness.GetParticipant("Async")->ComAdapter();
    auto* async = testHarness.GetParticipant("Async")->ComAdapter()->GetParticipantController();

    sync->SetSimulationTask([&syncTimeNs](auto now) {
        std::cout << "Sync SimTask now=" << now.count() << std::endl;
        syncTimeNs = now;
    });

    async->SetSimulationTaskAsync([&](auto now, auto) {
        std::cout << "Async SimTask now=" << now.count() 
            << " expectedTime=" << expectedTime.count()
            << std::endl;
        if (now == expectedTime)
        {
            std::cout << "Stopping simulation at expected time" << std::endl;
            asyncComAdapter->GetSystemController()->Stop();
        }
        if (now < expectedTime)
        {
            //Only allow time progress up to expectedTime
            async->CompleteSimulationTask();
        }
    });

    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach timeout";

    auto isSame = expectedTime == syncTimeNs;
    auto isOffByOne = syncTimeNs == (expectedTime + 1ms);
    ASSERT_TRUE(isSame || isOffByOne)
        << "Simulation time should be at most off by one expectedTime "
        << " (due to NextSimTask handling in distributed participants): "
        << " expectedTime=" << expectedTime.count()
        << " syncTime=" << syncTimeNs.count();
}

std::promise<bool> startupPromise;
std::promise<void> nextIterPromise;

auto BackgroundThread(ib::mw::sync::IParticipantController* parti)
{
    while (true)
    {
        auto start = startupPromise.get_future();
        start.wait();
        if (!start.valid())
        {
            std::cout << "Background thread terminating" << std::endl;
            return;
        }
        if (start.get())
        {
            std::cout << "Background thread terminating" << std::endl;
            return;
        }
        else
        {
            std::cout << "Calling CompleteSimulationTask from background thread" << std::endl;
            parti->CompleteSimulationTask();
            startupPromise = decltype(startupPromise){}; // reset for next iteration
            nextIterPromise.set_value();
        }
    }
}

TEST(AsyncSimTaskITest, test_async_simtask_completion_from_foreign_thread)
{
    // The async participant uses the ExecuteSimtaskNonBlocking and CompleteSimTask calls
    // The sync participant will be used to check the time progress

    SimTestHarness testHarness({"Sync", "Async"}, 42);

    auto syncTime{0ns};

    auto* sync = testHarness.GetParticipant("Sync")->ComAdapter()->GetParticipantController();
    auto* asyncComAdapter = testHarness.GetParticipant("Async")->ComAdapter();
    auto* async = testHarness.GetParticipant("Async")->ComAdapter()->GetParticipantController();

    sync->SetSimulationTask([&syncTime](auto now) {
        syncTime = now;
    });

    async->SetSimulationTaskAsync([&](auto now, auto) {
        if (now == expectedTime)
        {
            std::cout << "Stopping simulation at expected time" << std::endl;
            asyncComAdapter->GetSystemController()->Stop();
            startupPromise.set_value(true);
            return;
        }

        if (now < expectedTime)
        {
            //Signal other thread to call CompleteSimTask
            startupPromise.set_value(syncTime == expectedTime);
            //Wait until other thread has signaled
            auto nextIter = nextIterPromise.get_future();
            nextIter.wait();
            nextIter.get();
            nextIterPromise = decltype(nextIterPromise){};
        }
    });

    auto thread = std::thread{[&]() {
        BackgroundThread(async);
    }};

    ASSERT_TRUE(testHarness.Run(5s)) << "TestSim Harness should not reach time out";

    thread.join();
    auto isSame = expectedTime == syncTime;
    auto isOffByOne = (syncTime == (expectedTime + 1ms)) || (syncTime == (expectedTime - 1ms));
    ASSERT_TRUE(isSame || isOffByOne)
        << "Simulation time should be at most off by one expectedTime "
        << " (due to NextSimTask handling in distributed participants): "
        << " expectedTime=" << expectedTime.count()
        << " syncTime=" << syncTime.count();
}
} // anonymous namespace
