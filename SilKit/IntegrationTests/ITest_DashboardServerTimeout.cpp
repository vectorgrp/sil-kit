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
#include "ITestThreadSafeLogger.hpp"

#include "silkit/services/can/all.hpp"
#include "IParticipantInternal.hpp"
#include "silkit/config/all.hpp"

#include "CreateDashboard.hpp"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Core;
using namespace SilKit::Config;
using namespace SilKit::Services;

class DashboardTestHarness : public ITest_SimTestHarness
{
protected:
    DashboardTestHarness()
        : ITest_SimTestHarness()
        , _dashboardUri(MakeTestDashboardUri())
        , _participantConfig(ParticipantConfigurationFromString(R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Info"}]}})"))
    {
    }
    ~DashboardTestHarness() {}

protected:
    Orchestration::ITimeSyncService::SimulationStepHandler CreateSimulationStepHandler(
        const std::string& participantName, Orchestration::ILifecycleService* lifecycleService)
    {
        return [this, participantName, lifecycleService](auto now, auto) {
            if (now < 200ms)
            {
                return;
            }
            if (!_result)
            {
                _result = true;
                lifecycleService->Stop("Test done");
                Log() << "---   " << participantName << ": Sending Stop from";
            }
        };
    }

protected: // members
    std::string _dashboardUri;
    std::shared_ptr<IParticipantConfiguration> _participantConfig;
    bool _result{false};
};

TEST_F(DashboardTestHarness, dashboard_creationtimeout)
{
    SetupFromParticipantList({"CanWriter"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        _participantConfig, _registryUri, _dashboardUri,
        [this]() {
            {
                /////////////////////////////////////////////////////////////////////////
                // CanWriter
                /////////////////////////////////////////////////////////////////////////
                const auto participantName = "CanWriter";
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
                auto&& participant = simParticipant->Participant();
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
                (void)participant->CreateCanController("CanController1", "CAN1");

                timeSyncService->SetSimulationStepHandler(
                    CreateSimulationStepHandler(participantName, lifecycleService), 1ms);
            }
        auto ok = _simTestHarness->Run(5s);
        ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
        },
        std::chrono::seconds{5}, std::chrono::seconds{0});
}

TEST_F(DashboardTestHarness, dashboard_updatetimeout)
{
    SetupFromParticipantList({"CanWriter"});
    auto testResult = SilKit::Dashboard::RunDashboardTest(
        _participantConfig, _registryUri, _dashboardUri, [this]() {
            {
                /////////////////////////////////////////////////////////////////////////
                // CanWriter
                /////////////////////////////////////////////////////////////////////////
                const auto participantName = "CanWriter";
                auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
                auto&& participant = simParticipant->Participant();
                auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
                auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
                (void)participant->CreateCanController("CanController1", "CAN1");

                timeSyncService->SetSimulationStepHandler(
                    CreateSimulationStepHandler(participantName, lifecycleService), 1ms);
            }
            auto ok = _simTestHarness->Run(5s);
            ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
        },
        std::chrono::seconds{0}, std::chrono::seconds{5});
}

} //end namespace
