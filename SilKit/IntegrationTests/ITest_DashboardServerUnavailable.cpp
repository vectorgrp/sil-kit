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

#include <memory>
#include <chrono>
#include <iostream>

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"

#include "silkit/services/can/all.hpp"

#include "gtest/gtest.h"

#include "silkit/config/all.hpp"
#include "CreateDashboard.hpp"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::Can;

struct TestState
{
    std::vector<uint8_t> payloadBytes;

    CanFrame msg;

    //Reader variables
    bool result = false;
    size_t messageCount{0};
};

TEST_F(ITest_SimTestHarness, dashboard_server_unavailable)
{
    //Create a simulation setup with 2 participants
    SetupFromParticipantList({"CanReader", "CanWriter"});

    //Test Results
    auto state = std::make_shared<TestState>();

    //Test data
    const std::string payload = "Hallo Welt";

    state->payloadBytes.resize(payload.size());
    std::copy(payload.begin(), payload.end(), state->payloadBytes.begin());

    state->msg = CanFrame{};
    state->msg.canId = 123;
    state->msg.dataField = state->payloadBytes;

    auto dashboard =
        SilKit::Dashboard::CreateDashboard(SilKit::Config::ParticipantConfigurationFromString(
                                               R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Info"}]}})"),
                                                        _registryUri, MakeTestDashboardUri());

    //Set up the Sending and receiving participants
    {
        /////////////////////////////////////////////////////////////////////////
        // CanWriter
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "CanWriter";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& canController = participant->CreateCanController("CanController1", "CAN1");

        lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
            Log() << "---   " << participantName << ": Init called, setting baud rate and starting";
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        timeSyncService->SetSimulationStepHandler(
            [canController, state](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
                Log() << "---   CanWriter sending CanFrame";
                canController->SendFrame(state->msg, (void*)(intptr_t)(0xDEADBEEF));
            },
            1ms);
    }

    {
        /////////////////////////////////////////////////////////////////////////
        // CanReader
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "CanReader";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        (void)simParticipant->GetOrCreateTimeSyncService();
        auto&& canController = participant->CreateCanController("CanController1", "CAN1");

        lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
            Log() << participantName << ": Init called, setting baud rate and starting";
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        canController->AddFrameHandler([state, lifecycleService](auto, const Can::CanFrameEvent& frameEvent) {
            if (frameEvent.userContext == nullptr)
            {
                //Ignore the early test messages
                return;
            }
            if (state->messageCount++ == 10)
            {
                lifecycleService->Stop("Test done");
                Log() << "---   CanReader: Sending Stop from";
            }
            state->result = true;
        });
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    EXPECT_TRUE(state->result) << " Expecting a message";
}

} //end namespace
