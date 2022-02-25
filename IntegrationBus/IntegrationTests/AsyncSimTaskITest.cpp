// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

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

TEST(AsyncSimTaskITest, test_async_simtask_nodeadlock)
{
    // The async participant uses the ExecuteSimtaskNonBlocking and CompleteSimTask calls
    // The sync participant will be used to check the time progress

    SimTestHarness testHarness({"Sync", "Async"}, 42);

    const auto expectedTime{10ms};
    auto syncTime{0ns};

    auto* sync = testHarness.GetParticipant("Sync")->ComAdapter()->GetParticipantController();
    auto* asyncComAdapter = testHarness.GetParticipant("Async")->ComAdapter();
    auto* async = testHarness.GetParticipant("Async")->ComAdapter()->GetParticipantController();

    sync->SetSimulationTask([&syncTime](auto now) {
        syncTime = now;
    });

    async->SetSimulationTaskAsync([&](auto, auto) {
        if (syncTime == expectedTime)
        {
            std::cout << "Stopping simulation at expected time" << std::endl;
            asyncComAdapter->GetSystemController()->Stop();
        }
        async->CompleteSimulationTask();
    });

    testHarness.Run(5s);
    ASSERT_EQ(expectedTime, syncTime) << "Simulation time should be exactly expectedTime";
}

} // anonymous namespace
