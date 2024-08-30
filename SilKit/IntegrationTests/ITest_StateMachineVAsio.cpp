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
#include <cstdlib>
#include <thread>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

using namespace SilKit::Services::Orchestration;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class ITest_StateMachineVAsio : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD0(CommunicationReadyHandler, void());
        MOCK_METHOD0(StopHandler, void());
        MOCK_METHOD0(ShutdownHandler, void());
        MOCK_METHOD1(ParticipantStateHandler,
                     void(ParticipantState)); // Helper to only check for status.state; no longer part of the API
    };

protected:
    ITest_StateMachineVAsio() = default;

    auto SetTargetState(ParticipantState state)
    {
        _targetState = state;
        _targetStatePromise = std::promise<void>{};
        auto future = _targetStatePromise.get_future();
        if (_currentState == state)
        {
            // If we are already in the correct state, we have to set the promise immediately.
            // This happens if the ParticipantStateHandler is triggered before setting up the
            // expectation here.
            _targetStatePromise.set_value();
        }
        return future;
    }

    void ParticipantStateHandler(const ParticipantState& state)
    {
        std::cout << "current state = " << state << std::endl;
        callbacks.ParticipantStateHandler(state);
        _currentState = state;

        if (_currentState == _targetState)
            _targetStatePromise.set_value();
    }

protected:
    ParticipantState _targetState{ParticipantState::Invalid};
    std::promise<void> _targetStatePromise;
    ParticipantState _currentState;

    Callbacks callbacks;
};

TEST_F(ITest_StateMachineVAsio, DISABLED_vasio_state_machine)
{
    std::vector<std::string> syncParticipantNames{"TestUnit"};

    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // Setup Participant for TestController
    auto participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 "TestController", registryUri);

    auto systemController = SilKit::Experimental::Participant::CreateSystemController(participant.get());
    systemController->SetWorkflowConfiguration({syncParticipantNames});

    auto monitor = participant->CreateSystemMonitor();
    monitor->AddParticipantStatusHandler(
        [this](ParticipantStatus status) { this->ParticipantStateHandler(status.state); });

    // Setup Participant for Test Unit
    auto participantTestUnit =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "TestUnit", registryUri);
    auto* lifecycleService =
        participantTestUnit->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    lifecycleService->SetCommunicationReadyHandler(
        [&callbacks = callbacks]() { callbacks.CommunicationReadyHandler(); });
    timeSyncService->SetSimulationStepHandler([](auto /*now*/, auto /*duration*/) {}, 1ms);

    lifecycleService->SetStopHandler([&callbacks = callbacks]() { callbacks.StopHandler(); });
    lifecycleService->SetShutdownHandler([&callbacks = callbacks]() { callbacks.ShutdownHandler(); });

    std::string participantName = "TestUnit";

    EXPECT_CALL(callbacks, CommunicationReadyHandler()).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);

    // Perform the actual test
    auto stateReached = SetTargetState(ParticipantState::ServicesCreated);
    auto finalState = lifecycleService->StartLifecycle();
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Running);
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Stopped);
    lifecycleService->Stop("Test");
    auto status = stateReached.wait_for(5s);
    EXPECT_EQ(status, std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Shutdown);
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    ASSERT_EQ(finalState.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}

} // anonymous namespace
