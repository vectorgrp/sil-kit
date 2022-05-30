// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LifecycleService.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockParticipant.hpp"
#include "ParticipantConfiguration.hpp"
#include "SyncDatatypeUtils.hpp"
#include "TimeSyncService.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::util;

using ::ib::mw::test::DummyParticipant;

class MockTimeSync : public TimeSyncService
{
public:
    using TimeSyncService::TimeSyncService;

public:
    MOCK_METHOD(void, SetSimulationTask, (SimTaskT task), (override));
    MOCK_METHOD(void, SetSimulationTaskAsync, (SimTaskT task), (override));
    MOCK_METHOD(void, CompleteSimulationTask, (), (override));
    MOCK_METHOD(void, SetSimulationTask, (std::function<void(std::chrono::nanoseconds now)> task), (override));
    MOCK_METHOD(void, SetPeriod, (std::chrono::nanoseconds), (override));
    MOCK_METHOD(std::chrono::nanoseconds, Now, (), (override, const));
};

class MockParticipant : public DummyParticipant
{
public:
    MockParticipant() 
    { 
    }

public:
    MOCK_METHOD(TimeSyncService*, CreateTimeSyncService, (sync::LifecycleService*));
    MOCK_METHOD(void, SendIbMessage, (const IIbServiceEndpoint*, const ParticipantStatus& msg));

public:
};

class MockServiceDescriptor : public IIbServiceEndpoint
{
public:
    ServiceDescriptor serviceDescriptor;
    MockServiceDescriptor(EndpointAddress ea, std::string participantName)
    {
        ServiceDescriptor id = from_endpointAddress(ea);
        id.SetParticipantName(participantName);
        SetServiceDescriptor(id);
    }
    void SetServiceDescriptor(const ServiceDescriptor& _serviceDescriptor) override
    {
        serviceDescriptor = _serviceDescriptor;
    }
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override { return serviceDescriptor; }
};

// Factory method to create a ParticipantStatus matcher that checks the state field
auto AParticipantStatusWithState(ParticipantState expected)
{
    return MatcherCast<const ParticipantStatus&>(Field(&ParticipantStatus::state, expected));
}

class LifecycleServiceTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, ReinitializeHandler, ());
        MOCK_METHOD(void, StopHandler, ());
        MOCK_METHOD(void, ShutdownHandler, ());
        MOCK_METHOD(void, SimTask, (std::chrono::nanoseconds));
    };

protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{1, 1024};
    EndpointAddress addrP2{2, 1024};
    EndpointAddress masterAddr{3, 1027};
    MockServiceDescriptor p2Id{addrP2, "P2"};
    MockServiceDescriptor masterId{masterAddr, "Master"};

    MockParticipant participant;
    Callbacks callbacks;
    cfg::HealthCheck healthCheckConfig;


};

TEST_F(LifecycleServiceTest, start_stop_uncoordinated)
{
    LifecycleService lifecycleService(&participant, healthCheckConfig);
    MockTimeSync mockTimeSync(&participant, &lifecycleService, healthCheckConfig);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    auto descriptor = from_endpointAddress(addr);
    lifecycleService.SetServiceDescriptor(descriptor);

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetReinitializeHandler(bind_method(&callbacks, &Callbacks::ReinitializeHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, StopHandler())
        .Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1);

    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Initialized)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.ExecuteLifecycleNoSyncTime(false, false);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.Stop("");
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(LifecycleServiceTest, start_reeinitialize_stop_coordinated)
{
    LifecycleService lifecycleService(&participant, healthCheckConfig);
    MockTimeSync mockTimeSync(&participant, &lifecycleService, healthCheckConfig);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    auto descriptor = from_endpointAddress(addr);
    lifecycleService.SetServiceDescriptor(descriptor);

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetReinitializeHandler(bind_method(&callbacks, &Callbacks::ReinitializeHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, StopHandler())
        .Times(2);
    EXPECT_CALL(callbacks, ReinitializeHandler())
        .Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1);

    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Initialized)))
        .Times(2);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(2);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(2);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopped)))
        .Times(2);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Reinitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.ExecuteLifecycleNoSyncTime(true, true);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // run & stop
    SystemCommand runCommand{SystemCommand::Kind::Run};
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    lifecycleService.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Stopped);
    // stopping again should work -> NOP
    lifecycleService.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Stopped);
    // reinitialize
    ParticipantCommand reinitializeCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Reinitialize};
    lifecycleService.ReceiveIbMessage(&masterId, reinitializeCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // run & stop again
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    lifecycleService.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Stopped);
    // shutdown
    ParticipantCommand shutdownCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Shutdown};
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(LifecycleServiceTest, error_handling_run_run_shutdown)
{
    LifecycleService lifecycleService(&participant, healthCheckConfig);
    MockTimeSync mockTimeSync(&participant, &lifecycleService, healthCheckConfig);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    auto descriptor = from_endpointAddress(addr);
    lifecycleService.SetServiceDescriptor(descriptor);

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetReinitializeHandler(bind_method(&callbacks, &Callbacks::ReinitializeHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, StopHandler())
        .Times(0);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(0);

    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Initialized)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(1);

    lifecycleService.ExecuteLifecycleNoSyncTime(true, true);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // run & stop
    SystemCommand runCommand{SystemCommand::Kind::Run};
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // repeat signal (must be ignored)
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // shutdown (invalid transition -> error)
    ParticipantCommand shutdownCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Shutdown};
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
}

TEST_F(LifecycleServiceTest, error_handling_error_recovery_reinitialize)
{
    LifecycleService lifecycleService(&participant, healthCheckConfig);
    MockTimeSync mockTimeSync(&participant, &lifecycleService, healthCheckConfig);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    auto descriptor = from_endpointAddress(addr);
    lifecycleService.SetServiceDescriptor(descriptor);

    lifecycleService.SetReinitializeHandler(bind_method(&callbacks, &Callbacks::ReinitializeHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, ReinitializeHandler())
        .Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1);

    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Initialized)))
        .Times(2);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(2);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Reinitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.ExecuteLifecycleNoSyncTime(true, true);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // run & stop
    SystemCommand runCommand{SystemCommand::Kind::Run};
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // repeat signal (must be ignored)
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // shutdown (invalid transition -> error)
    ParticipantCommand shutdownCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Shutdown};
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    // recover via reinitialize
    ParticipantCommand reinitializeCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Reinitialize};
    lifecycleService.ReceiveIbMessage(&masterId, reinitializeCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // break it again
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    // recover via shutdown
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}

TEST_F(LifecycleServiceTest, error_handling_exception_in_callback)
{
    LifecycleService lifecycleService(&participant, healthCheckConfig);
    MockTimeSync mockTimeSync(&participant, &lifecycleService, healthCheckConfig);
    lifecycleService.SetTimeSyncService(&mockTimeSync);
    ON_CALL(participant, CreateTimeSyncService(_)).WillByDefault(Return(&mockTimeSync));

    auto descriptor = from_endpointAddress(addr);
    lifecycleService.SetServiceDescriptor(descriptor);

    lifecycleService.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    lifecycleService.SetReinitializeHandler(bind_method(&callbacks, &Callbacks::ReinitializeHandler));
    lifecycleService.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    EXPECT_CALL(callbacks, StopHandler())
        .Times(1)
        .WillRepeatedly(Throw(std::runtime_error("StopCallbackException")));
    EXPECT_CALL(callbacks, ReinitializeHandler())
        .Times(1)
        .WillRepeatedly(Throw(std::runtime_error("ReinitializeCallbackException")));
    EXPECT_CALL(callbacks, ShutdownHandler())
        .Times(1)
        .WillRepeatedly(Throw(std::runtime_error("ShutdownCallbackException")));

    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Initialized)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Running)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Stopping)))
        .Times(1);
    EXPECT_CALL(participant, 
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Error)))
        .Times(2);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Reinitializing)))
        .Times(1);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::ShuttingDown)))
        .Times(1);
    EXPECT_CALL(participant,
                SendIbMessage(&lifecycleService, AParticipantStatusWithState(ParticipantState::Shutdown)))
        .Times(1);

    lifecycleService.ExecuteLifecycleNoSyncTime(true, true);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Initialized);
    // run
    SystemCommand runCommand{SystemCommand::Kind::Run};
    lifecycleService.ReceiveIbMessage(&masterId, runCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Running);
    // stop - callback throws -> expect error state
    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    lifecycleService.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    // recover via reinitialize - callback throws -> expect error state
    ParticipantCommand reinitializeCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Reinitialize};
    lifecycleService.ReceiveIbMessage(&masterId, reinitializeCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Error);
    // recover via shutdown - callback throws -> expect shutdown state (error will be ignored)
    ParticipantCommand shutdownCommand{descriptor.GetParticipantId(), ParticipantCommand::Kind::Shutdown};
    lifecycleService.ReceiveIbMessage(&masterId, shutdownCommand);
    EXPECT_EQ(lifecycleService.State(), ParticipantState::Shutdown);
}
} // namespace