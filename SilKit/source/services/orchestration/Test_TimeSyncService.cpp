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

#include "TimeSyncService.hpp"
#include "MockParticipant.hpp"
#include "ParticipantConfiguration.hpp"
#include "SyncDatatypeUtils.hpp"
#include "TimeSyncService.hpp"
#include "LifecycleService.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;


class MockServiceEndpoint : public IServiceEndpoint
{
public:
    MockServiceEndpoint(EndpointAddress ea, std::string participantName)
    {
        _serviceDescriptor = from_endpointAddress(ea);
        _serviceDescriptor.SetParticipantName(std::move(participantName));

        ON_CALL(*this, GetServiceDescriptor()).WillByDefault(ReturnRef(_serviceDescriptor));
    }
    MOCK_METHOD(void,SetServiceDescriptor, (const ServiceDescriptor& ), (override));
    MOCK_METHOD(const ServiceDescriptor&, GetServiceDescriptor, (),(const,override));

    ServiceDescriptor _serviceDescriptor;
};


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
        : timeSyncService{&participant, participant.GetTimeProvider(), healthCheckConfig}
    {
        // Fix the dependencies between lifecycle and timesync services:
        ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault([this](auto arg) {
            timeSyncService.SetLifecycleService(arg);
            return &timeSyncService;
        });

        // this CTor calls CreateTimeSyncService implicitly
        lifecycleService = std::make_unique<LifecycleService>(&participant);
    }
protected: // Methods
    void PrepareLifecycle()
    {
        lifecycleService->SetTimeSyncActive(true);
        (void)lifecycleService->StartLifecycle({true, true});
        // skip uninteresting states
        lifecycleService->NewSystemState(SystemState::ServicesCreated);
        lifecycleService->NewSystemState(SystemState::CommunicationInitializing);
        lifecycleService->NewSystemState(SystemState::CommunicationInitialized);
        lifecycleService->NewSystemState(SystemState::ReadyToRun);
        lifecycleService->NewSystemState(SystemState::Running);
        lifecycleService->ReceiveMsg(&endpoint, {SystemCommand::Kind::Run});
    }
protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{1, 1024};
    MockServiceEndpoint endpoint{addr, "P1"};

    DummyParticipant participant;
    Callbacks callbacks;
    Config::HealthCheck healthCheckConfig;

    std::unique_ptr<LifecycleService> lifecycleService;
    TimeSyncService timeSyncService;
};

auto makeTask(std::chrono::milliseconds timepoint)
{
    //XXX remove this once we get rid of VS2015, initalizer_list is broken there
    NextSimTask task;
    task.timePoint = timepoint;
    task.duration = 0ms;
    return task;
}

TEST_F(TimeSyncServiceTest, async_simtask_once_without_complete_call)
{
    PrepareLifecycle();

    auto numAsyncTaskCalled{0};
    timeSyncService.SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    auto task =  makeTask(0ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    task = makeTask(1ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    task = makeTask(2ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    ASSERT_EQ(numAsyncTaskCalled, 1)
        << "SimulationStepHandlerAsync should only be called once"
        << " until completed with a call to CompleteSimulationStep().";
}

TEST_F(TimeSyncServiceTest, async_simtask_complete_lockstep)
{
    PrepareLifecycle();

    auto numAsyncTaskCalled{0};
    timeSyncService.SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    auto task = makeTask(0ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();

    task = makeTask(1ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();

    task = makeTask(2ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();
    ASSERT_EQ(numAsyncTaskCalled, 3)
        << "SimulationStepHandlerAsync should be called in lockstep with calls to CompleteSimulationStep().";
}

TEST_F(TimeSyncServiceTest, async_simtask_mismatching_number_of_complete_calls)
{
    // What happens when the User calls CompleteSimulationStep() multiple times?
    PrepareLifecycle();

    auto numAsyncTaskCalled{0};
    timeSyncService.SetSimulationStepHandlerAsync([&](auto, auto){
        numAsyncTaskCalled++;
    }, 1ms);

    auto task = makeTask(0ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();
    timeSyncService.CompleteSimulationStep();

    task = makeTask(1ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();
    timeSyncService.CompleteSimulationStep();

    task = makeTask(2ms);
    timeSyncService.ReceiveMsg(&endpoint, task);
    timeSyncService.CompleteSimulationStep();
    timeSyncService.CompleteSimulationStep();

    ASSERT_EQ(numAsyncTaskCalled, 3)
        << "Calling too many CompleteSimulationStep() should not wreak havoc"; 
}

} // namespace
