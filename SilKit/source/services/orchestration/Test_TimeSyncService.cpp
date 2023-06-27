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
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/orchestration/string_utils.hpp"
#include "functional.hpp"

#include "MockParticipant.hpp"
#include "MockServiceEndpoint.hpp"
#include "ParticipantConfiguration.hpp"
#include "SyncDatatypeUtils.hpp"
#include "TimeSyncService.hpp"
#include "LifecycleService.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;


class TimeSyncServiceTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, CommunicationReadyHandler, ());
        MOCK_METHOD(void, StartingHandler, ());
        MOCK_METHOD(void, StopHandler, ());
        MOCK_METHOD(void, ShutdownHandler, ());
        MOCK_METHOD(void, SimTask, (std::chrono::nanoseconds));
    };

protected: // CTor
    TimeSyncServiceTest()
    {
        // this CTor calls CreateTimeSyncService implicitly
        lifecycleService =
            std::make_unique<LifecycleService>(&participant);
        lifecycleService->SetLifecycleConfiguration(LifecycleConfiguration{OperationMode::Coordinated});
        timeSyncService =
            std::make_unique<TimeSyncService>(&participant, &timeProvider, healthCheckConfig, lifecycleService.get());
        lifecycleService->SetTimeSyncService(timeSyncService.get());
    }
protected: // Methods
    void PrepareLifecycle()
    {
        lifecycleService->SetTimeSyncActive(true);
        (void)lifecycleService->StartLifecycle();
        
        // Add other participant to lookup
        timeSyncService->GetTimeConfiguration()->AddSynchronizedParticipant("P1");
        
        // skip uninteresting states
        lifecycleService->NewSystemState(SystemState::ServicesCreated);
        lifecycleService->NewSystemState(SystemState::CommunicationInitializing);
        lifecycleService->NewSystemState(SystemState::CommunicationInitialized);
        lifecycleService->NewSystemState(SystemState::ReadyToRun);
        lifecycleService->NewSystemState(SystemState::Running);
    }
protected:
    // ----------------------------------------
    // Members
    NiceMock<MockServiceEndpoint> endpoint{"P1", "N1", "C1"};

    NiceMock<DummyParticipant> participant;
    Callbacks callbacks;
    Config::HealthCheck healthCheckConfig;

    std::unique_ptr<LifecycleService> lifecycleService;
    TimeProvider timeProvider{};
    std::unique_ptr<TimeSyncService> timeSyncService;
};

TEST_F(TimeSyncServiceTest, async_simtask_once_without_complete_call)
{
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    ASSERT_EQ(numAsyncTaskCalled, 1)
        << "SimulationStepHandlerAsync should only be called once"
        << " until completed with a call to CompleteSimulationStep().";
}

TEST_F(TimeSyncServiceTest, async_simtask_complete_lockstep)
{
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->CompleteSimulationStep();
    
    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->CompleteSimulationStep();
    
    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    timeSyncService->CompleteSimulationStep();
    ASSERT_EQ(numAsyncTaskCalled, 3)
        << "SimulationStepHandlerAsync should be called in lockstep with calls to CompleteSimulationStep().";
}

TEST_F(TimeSyncServiceTest, async_simtask_mismatching_number_of_complete_calls)
{
    // What happens when the User calls CompleteSimulationStep() multiple times?
    auto numAsyncTaskCalled{0};
    timeSyncService->SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    PrepareLifecycle();

    timeSyncService->ReceiveMsg(&endpoint, {0ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {1ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    timeSyncService->ReceiveMsg(&endpoint, {2ms});
    timeSyncService->CompleteSimulationStep();
    timeSyncService->CompleteSimulationStep();

    ASSERT_EQ(numAsyncTaskCalled, 3)
        << "Calling too many CompleteSimulationStep() should not wreak havoc"; 
}

} // namespace
