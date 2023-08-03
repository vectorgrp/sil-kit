/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include "gtest/gtest.h"

#include "ITestFixture.hpp"

#include "silkit/services/can/all.hpp"
#include "IParticipantInternal.hpp"
#include "silkit/config/all.hpp"

#include "CreateDashboard.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace std::chrono_literals;

struct ITest_DashboardServerTimeout : ITest_DashboardTestHarness
{
};

TEST_F(ITest_DashboardServerTimeout, dashboard_creationtimeout)
{
    const auto participantName = "CanWriter";
    const auto canonicalName = "CanController1";
    const auto networkName = "CAN1";
    SetupFromParticipantLists({participantName}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName, &canonicalName, &networkName]() {
            _simTestHarness->CreateSystemController();
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName, _participantConfig);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateCanController(canonicalName, networkName);
            timeSyncService->SetSimulationStepHandler(
                [lifecycleService](auto, auto) {
                    lifecycleService->Stop("Test done");
                },
                1ms);
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
            _simTestHarness->ResetParticipants();
        },
        1, std::chrono::seconds{7}, std::chrono::seconds{0});
    ASSERT_FALSE(testResult.allSimulationsFinished) << "Simulation should not be finished!";
    _simTestHarness->ResetRegistry();
}

TEST_F(ITest_DashboardServerTimeout, dashboard_updatetimeout)
{
    const auto participantName = "CanWriter";
    const auto canonicalName = "CanController1";
    const auto networkName = "CAN1";
    SetupFromParticipantLists({participantName}, {});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        ParticipantConfigurationFromStringImpl(_dashboardParticipantConfig), _registryUri, _dashboardUri,
        [this, &participantName, &canonicalName, &networkName]() {
            _simTestHarness->CreateSystemController();
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName, _participantConfig);
            auto&& participant = simParticipant->Participant();
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
            (void)participant->CreateCanController(canonicalName, networkName);
            timeSyncService->SetSimulationStepHandler(
                [lifecycleService](auto, auto) {
                    lifecycleService->Stop("Test done");
                },
                1ms);
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
            _simTestHarness->ResetParticipants();
        },
        1, std::chrono::seconds{0}, std::chrono::seconds{7});
    ASSERT_FALSE(testResult.allSimulationsFinished) << "Simulation should not be finished!";
    _simTestHarness->ResetRegistry();
}

} //end namespace
