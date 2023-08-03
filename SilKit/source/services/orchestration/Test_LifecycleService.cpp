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

#include "LifecycleService.hpp"
#include "MockParticipant.hpp"
#include "MockServiceEndpoint.hpp"
#include "ParticipantConfiguration.hpp"
#include "SyncDatatypeUtils.hpp"
#include "TimeSyncService.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class MockTimeSync : public TimeSyncService
{
public:
    using TimeSyncService::TimeSyncService;

public:
    MOCK_METHOD(void, SetSimulationStepHandler, (SimulationStepHandler task, std::chrono::nanoseconds initialStepSize), (override));
    MOCK_METHOD(void, SetSimulationStepHandlerAsync, (SimulationStepHandler task, std::chrono::nanoseconds initialStepSize),
                (override));
    MOCK_METHOD(void, CompleteSimulationStep, (), (override));
    MOCK_METHOD(void, SetPeriod, (std::chrono::nanoseconds));
    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
};

class MockParticipant : public DummyParticipant
{
public:
    MockParticipant() 
    { 
    }

public:
    MOCK_METHOD(TimeSyncService*, CreateTimeSyncService, (LifecycleService*));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const ParticipantStatus& msg));

public:
};


// Factory method to create a ParticipantStatus matcher that checks the state field
auto AParticipantStatusWithState(ParticipantState expected)
{
    return MatcherCast<const ParticipantStatus&>(Field(&ParticipantStatus::state, expected));
}

class Test_LifecycleService : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, CommunicationReadyHandler, ());
        MOCK_METHOD(void, StartingHandler, ());
        MOCK_METHOD(void, StopHandler, ());
        MOCK_METHOD(void, ShutdownHandler, ());
        MOCK_METHOD(void, AbortHandler, (ParticipantState));
        MOCK_METHOD(void, SimTask, (std::chrono::nanoseconds));
    };

protected:
    // ----------------------------------------
    // Members
    NiceMock<MockServiceEndpoint> p1Id{"P1","N1", "C1", 1024};
    NiceMock<MockServiceEndpoint> p2Id{"P2","N1", "C1", 1024};
    NiceMock<MockServiceEndpoint> masterId{"Master", "N1", "C2", 1027};

    NiceMock<MockParticipant> participant;
    Callbacks callbacks;
    Config::HealthCheck healthCheckConfig;
};

// will forward lifecycle with coordination up to 
// ParticipantState::ReadyToRun & SystemState::ReadyToRun
void PrepareLifecycle(LifecycleService* lifecycleService)
{
    lifecycleService->NewSystemState(SystemState::ServicesCreated);
    lifecycleService->NewSystemState(SystemState::CommunicationInitializing);
    lifecycleService->NewSystemState(SystemState::CommunicationInitialized);
    lifecycleService->NewSystemState(SystemState::ReadyToRun);
}

auto StartCoordinated() 
{
    LifecycleConfiguration sc{};
    sc.operationMode = OperationMode::Coordinated;
    return sc;
}

auto StartAutonomous()
{
    LifecycleConfiguration sc;
    sc.operationMode = OperationMode::Autonomous;
    return sc;
}

TEST_F(Test_LifecycleService, autonomous_must_not_react_to_system_states)
{
    LifecycleConfiguration lc{OperationMode::Autonomous};
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(lc);
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService,
                              AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService,
                              AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(0);

    lifecycleService.StartLifecycle();
    // coordinated participants stay in ServicesCreated, autonomous advance to running
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // participant must not change state upon receiving system states
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::ReadyToRun);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Running);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Invalid);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Stopping);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Stopped);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::ShuttingDown);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Shutdown);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Error);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
}

TEST_F(Test_LifecycleService, start_stop_autonomous)
{
    LifecycleConfiguration lc{OperationMode::Autonomous};
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(lc);
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    lifecycleService.SetCommunicationReadyHandler(bind_method(&callbacks, &Callbacks::CommunicationReadyHandler));
    lifecycleService.SetStartingHandler(bind_method(&callbacks, &Callbacks::StartingHandler));
    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    
    EXPECT_CALL(callbacks, CommunicationReadyHandler()).Times(1);
    EXPECT_CALL(callbacks, StartingHandler()).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);

    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.StartLifecycle();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Stop("");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, start_stop_coordinated_self_stop)
{
    // Intended state order: Create, ..., start, stop, create, start, stop, shutdown
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    lifecycleService.SetCommunicationReadyHandler(bind_method(&callbacks, &Callbacks::CommunicationReadyHandler));
    lifecycleService.SetStartingHandler(bind_method(&callbacks, &Callbacks::StartingHandler));
    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    
    EXPECT_CALL(callbacks, CommunicationReadyHandler())
        .Times(1);
    EXPECT_CALL(callbacks, StartingHandler())
        .Times(1);
    EXPECT_CALL(callbacks, StopHandler())
        .Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1);

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService,
                              AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService,
                              AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(2);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Paused)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.StartLifecycle();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::CommunicationInitialized);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::ReadyToRun);
    lifecycleService.NewSystemState(SystemState::ReadyToRun);
    // run, pause & stop
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Pause("Test");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Paused);
    lifecycleService.Continue();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Stop("Manual Stop");
    //lifecycleService.NewSystemState(SystemState::Stopped);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, start_stop_coordinated_external_stop)
{
    // Intended state order: Create, ..., start, stop, create, start, stop, shutdown
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    lifecycleService.SetCommunicationReadyHandler(bind_method(&callbacks, &Callbacks::CommunicationReadyHandler));
    lifecycleService.SetStartingHandler(bind_method(&callbacks, &Callbacks::StartingHandler));
    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, CommunicationReadyHandler()).Times(1);
    EXPECT_CALL(callbacks, StartingHandler()).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);

    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(2);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Paused)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.StartLifecycle();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::CommunicationInitialized);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::ReadyToRun);
    lifecycleService.NewSystemState(SystemState::ReadyToRun);
    // run, pause & stop
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Pause("Test");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Paused);
    lifecycleService.Continue();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Stopping);
    //lifecycleService.NewSystemState(SystemState::Stopped);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, error_on_double_pause)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Paused)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Pause("Test");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Paused);
    EXPECT_THROW({ lifecycleService.Pause("Pause again"); }, SilKitError);
}



TEST_F(Test_LifecycleService, error_handling_run_run_shutdown)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, StopHandler())
        .Times(0);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(0);

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    // run & stop
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // shutdown (invalid transition -> error)
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
}

TEST_F(Test_LifecycleService, error_handling_exception_in_stop_callback)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));

    EXPECT_CALL(callbacks, StopHandler()).Times(1).WillRepeatedly(Throw(SilKitError("StopCallbackException")));

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    // run
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // stop - callback throws -> expect error state
    lifecycleService.NewSystemState(SystemState::Stopping);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    // recover via abortSimulation - callback throws -> finally expect shutdown state (error will be ignored)
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_BeforeLifecycleStart)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));
    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));

    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Invalid);
    }));
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    // Receive abort...
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    // ...than call StartLifecycle
    auto finalStateFuture = lifecycleService.StartLifecycle();
    // Final state must be reached
    auto finalState = finalStateFuture.get();
    // Abort must result in Shutdown state
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
    EXPECT_EQ(finalState, ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_AfterLifecycleStart)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));
    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));

    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Running);
    }));
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    // Call StartLifecycle...
    auto finalStateFuture = lifecycleService.StartLifecycle();
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::CommunicationInitializing);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    lifecycleService.NewSystemState(SystemState::ReadyToRun);

    // ...then receive abort
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);

    // Final state must be reached
    auto finalState = finalStateFuture.get();
    // Abort must result in Shutdown state
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
    EXPECT_EQ(finalState, ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_CommunicationReady_Callback)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetCommunicationReadyHandler(bind_method(&callbacks, &Callbacks::CommunicationReadyHandler));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(callbacks, CommunicationReadyHandler()).Times(1).WillRepeatedly(Invoke([&]() {
        SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
        lifecycleService.ReceiveMsg(&masterId, abortCommand);
        EXPECT_EQ(lifecycleService.State(), ParticipantState::CommunicationInitialized);
    }));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::CommunicationInitialized);
    }));

    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::CommunicationInitializing);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_ReadyToRun)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::ReadyToRun);
    }));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::CommunicationInitializing);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::ReadyToRun);
    // Abort right away
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_Starting)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartAutonomous());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetStartingHandler(bind_method(&callbacks, &Callbacks::StartingHandler));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::ReadyToRun);
    }));

    EXPECT_CALL(callbacks, StartingHandler()).Times(1).WillRepeatedly(Invoke([&]() {
        SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
        lifecycleService.ReceiveMsg(&masterId, abortCommand);
        EXPECT_EQ(lifecycleService.State(), ParticipantState::ReadyToRun);
    }));

    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_Running)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Running);
    }));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    // run
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // Abort right away
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_Paused)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Paused);
    }));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Paused)))
        .Times(1);
    // Note: if aborting during Running, the simulation will stop regularly (including the stopping command), 
    // but without stopping if there are errors in the callbacks
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // run
    lifecycleService.Pause("Test");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Paused);
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}


TEST_F(Test_LifecycleService, Abort_Stopping)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Stopping);
    }));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(callbacks, StopHandler())
        .Times(1)
        .WillRepeatedly(Invoke([&]() 
            {
                SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
                lifecycleService.ReceiveMsg(&masterId, abortCommand);
                EXPECT_EQ(lifecycleService.State(), ParticipantState::Stopping);
            }));
    
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.NewSystemState(SystemState::Stopping);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_ShuttingDown)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(0);

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1)
        .WillRepeatedly(Invoke([&]() 
            {
                SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
                lifecycleService.ReceiveMsg(&masterId, abortCommand);
                EXPECT_EQ(lifecycleService.State(), ParticipantState::ShuttingDown);
            }));

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(0);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // running
    lifecycleService.NewSystemState(SystemState::Stopping);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_Shutdown)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(0);

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // running
    lifecycleService.NewSystemState(SystemState::Stopping);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
    // Abort right away
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_Aborting)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(1).WillRepeatedly(Invoke([&](auto participantState) {
        EXPECT_EQ(participantState, ParticipantState::Running);
        SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
        lifecycleService.ReceiveMsg(&masterId, abortCommand);
    }));

    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Aborting)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);

    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);

    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(Test_LifecycleService, Abort_LifecycleNotExecuted)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    // Abort callback registration
    lifecycleService.SetAbortHandler(bind_method(&callbacks, &Callbacks::AbortHandler));
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(0);

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(0);
    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(0);

    EXPECT_EQ(lifecycleService.State(), ParticipantState::Invalid);
    // Abort right away
    SystemCommand abortCommand{SystemCommand::Kind::AbortSimulation};
    lifecycleService.ReceiveMsg(&masterId, abortCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Invalid);
}

TEST_F(Test_LifecycleService, error_handling_exception_in_starting_callback)
{
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetStartingHandler(bind_method(&callbacks, &Callbacks::StartingHandler));
    EXPECT_CALL(callbacks, StartingHandler())
        .Times(1)
        .WillRepeatedly(Throw(SilKitError("StartingException")));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    EXPECT_CALL(participant,
                SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::ServicesCreated)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitializing)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::CommunicationInitialized)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService,
                                           AParticipantStatusWithState(ParticipantState::ReadyToRun)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(1);

    lifecycleService.StartLifecycle();
    PrepareLifecycle(&lifecycleService);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
}

TEST_F(Test_LifecycleService, async_comm_ready_handler)
{
    // Goal: ensure that the basic calls to SetCommunicationReadyHandlerAsync() and
    // CompleteCommunicationReadyHandler() are working as expected.
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    lifecycleService.SetServiceDescriptor(p1Id.GetServiceDescriptor());

    std::promise<void> myPromise;
    auto completed = myPromise.get_future();

    // Calling this before the CommunicationReadyHandler has been invoked should not result in an error:
    lifecycleService.CompleteCommunicationReadyHandlerAsync();

    lifecycleService.SetCommunicationReadyHandlerAsync([&myPromise] {
        std::cout <<"async_comm_ready_handler: signaling myPromise" << std::endl;
        myPromise.set_value();
    });


    std::thread completer{[&completed, &lifecycleService]{
        std::cout <<"async_comm_ready_handler: completer waiting"
            " after call to CompleteCommReadyHandler" << std::endl;
        auto status = completed.wait_for(5s);
        EXPECT_EQ(status, std::future_status::ready)
            << "Error: CommunicationReadyHandler async should have been called";

        std::cout <<"async_comm_ready_Handler: completer calling CommunicationReadyHandler 1st" << std::endl;
        lifecycleService.CompleteCommunicationReadyHandlerAsync();
        
        // Calling this multiple times should not result in an error:
        std::cout <<"async_comm_ready_Handler: completer calling CommunicationReadyHandler 2nd" << std::endl;
        lifecycleService.CompleteCommunicationReadyHandlerAsync();

    }};

    lifecycleService.StartLifecycle();

    lifecycleService.NewSystemState(SystemState::ServicesCreated);
    lifecycleService.NewSystemState(SystemState::CommunicationInitializing);
    lifecycleService.NewSystemState(SystemState::CommunicationInitialized);

    if(completer.joinable())
    {
        completer.join();
    }
    ASSERT_EQ(lifecycleService.State(), ParticipantState::ReadyToRun);
}

TEST_F(Test_LifecycleService, error_on_create_time_sync_service_twice)
{
    // Goal: make sure that CreateTimeSync cannot be called more than once (must throw exception)
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());

    EXPECT_NO_THROW({
        try
        {
            lifecycleService.CreateTimeSyncService(); // ignore returned controller
        }
        catch (const std::exception&)
        {
            throw;
        }
    });

    EXPECT_THROW(
        {
            try
            {
                lifecycleService.CreateTimeSyncService(); // ignore returned controller
            }
            catch (const ConfigurationError&)
            {
                throw;
            }
        },
        ConfigurationError);
}

TEST_F(Test_LifecycleService, error_on_coordinated_not_required)
{
    // Goal: make sure that the lifecycleService throws an exception if it is first 
    // set to be coordinated and then receives a required participant list without its own name
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Invalid);

    WorkflowConfiguration workflowConfiguration;
    workflowConfiguration.requiredParticipantNames = {"NotThisParticipant", "AlsoNotThisParticipant"};
    lifecycleService.SetWorkflowConfiguration(workflowConfiguration);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Invalid);
}

TEST_F(Test_LifecycleService, error_on_not_required_coordinated)
{
    // Goal: make sure that the lifecycleService throws an exception if it first 
    // receives a required participant list without its own name and is then set to be coordinated
    LifecycleService lifecycleService(&participant);
    lifecycleService.SetLifecycleConfiguration(StartCoordinated());
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Invalid);

    MockTimeSync mockTimeSync(&participant, &participant.mockTimeProvider, healthCheckConfig, &lifecycleService);
    lifecycleService.SetTimeSyncService(&mockTimeSync);

    lifecycleService.StartLifecycle();
    
    WorkflowConfiguration workflowConfiguration;
    workflowConfiguration.requiredParticipantNames = {"NotThisParticipant", "AlsoNotThisParticipant"};
    lifecycleService.SetWorkflowConfiguration(workflowConfiguration);

    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    
}
} // namespace
