// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "SystemMonitor.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/util/functional.hpp"

#include "MockParticipant.hpp"
#include "SyncDatatypeUtils.hpp"
#include "ib/mw/sync/string_utils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::util;

using ::ib::mw::test::DummyParticipant;

class SystemMonitorTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(SystemStateHandler, void(SystemState));
        MOCK_METHOD1(ParticipantStatusHandler, void(ParticipantStatus));

        MOCK_METHOD(void, ParticipantConnectedHandler, (const std::string &), (const));
        MOCK_METHOD(void, ParticipantDisconnectedHandler, (const std::string &), (const));
    };

protected:

    SystemMonitorTest()
        : monitor{&participant }
        , monitorFrom{ &participant }
    {
        syncParticipantNames = { "P1", "P2", "P3" };
        monitor.UpdateExpectedParticipantNames(ib::mw::sync::ExpectedParticipants{ syncParticipantNames });
        monitor.SetServiceDescriptor(from_endpointAddress(addr));
    }

    void RegisterSystemHandler()
    {
        monitor.RegisterSystemStateHandler(bind_method(&callbacks, &Callbacks::SystemStateHandler));
    }
    void RegisterParticipantStatusHandler()
    {
        monitor.RegisterParticipantStatusHandler(bind_method(&callbacks, &Callbacks::ParticipantStatusHandler));
    }

    void SetParticipantStatus(ParticipantId participantId, ParticipantState state, std::string reason = std::string{})
    {
        uint64_t id = participantId - 1;
        ParticipantStatus status;
        status.state = state;
        status.participantName = syncParticipantNames.at(static_cast<size_t>(id));
        status.enterReason = reason;

        EndpointAddress from;
        from.participant = id;
        from.endpoint = 1024;

        monitorFrom.SetServiceDescriptor(from_endpointAddress(from));
        
        monitor.ReceiveIbMessage(&monitorFrom, status);
    }

    void SetAllParticipantStates(ParticipantState state)
    {
        for (size_t i = 0; i < syncParticipantNames.size(); i++)
        {
            SetParticipantStatus(i+1, state);
        }
        EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
    }

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{19, 1025};

    std::vector<std::string> syncParticipantNames;

    DummyParticipant participant;
    SystemMonitor monitor;
    SystemMonitor monitorFrom;
    Callbacks callbacks;
};

TEST_F(SystemMonitorTest, init_with_state_invalid)
{
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Invalid);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Invalid);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Invalid);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_controllers_created)
{
    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ControllersCreated)).Times(1);

    SetParticipantStatus(1, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ControllersCreated);

    SetParticipantStatus(2, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::ControllersCreated);

    SetParticipantStatus(3, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::ControllersCreated);

    EXPECT_EQ(monitor.SystemState(), SystemState::ControllersCreated);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_initializing)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(2, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_initialized)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Initialized);

    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_running)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    ASSERT_EQ(monitor.SystemState(), SystemState::Initialized);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    SetParticipantStatus(2, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    SetParticipantStatus(3, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Running);

    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}


TEST_F(SystemMonitorTest, detect_system_pause)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused))
        .Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_multiple_paused_clients)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused))
        .Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(2, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(2, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_stopping)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_stopped)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping))
        .Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopped))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(1, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(2, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(2, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(3, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(3, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initializing_after_stopped)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initialized_after_stopped)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ShuttingDown))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shutdown)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);


    SetParticipantStatus(1, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    SetParticipantStatus(2, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown))
        .Times(1);
    SetParticipantStatus(3, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_controllers_created)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ControllersCreated);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initializing)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initialized)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_running)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_paused)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopping)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopped)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    // if the Shutdown callback triggers an error, this can lead to a temporary SystemError state.
    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    // After the callback, the participant state will be set to Shutdown.
    // The system state will remain in Error until all participants are Shutdown.
    SetParticipantStatus(1, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown))
        .Times(1);
    SetAllParticipantStates(ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initializing_after_error)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ControllersCreated);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_one_swapping)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);

    RegisterSystemHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPending)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(2, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(3, ParticipantState::ColdswapShutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapDone)).Times(1);
    SetParticipantStatus(3, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_all_swapping)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);

    RegisterSystemHandler();

    // Shutdown participants one after another...
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPending)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapShutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(2, ParticipantState::ColdswapShutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(3, ParticipantState::ColdswapShutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    // Bring participants back online...
    SetParticipantStatus(1, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(2, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapDone)).Times(1);
    SetParticipantStatus(3, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_none_swapping)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);

    RegisterSystemHandler();

    // Ignore coldswap for all participants
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPending)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(2, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapDone)).Times(1);
    SetParticipantStatus(3, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);
}


TEST_F(SystemMonitorTest, detect_initializing_after_coldswapdone)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);
    SetAllParticipantStates(ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);

    RegisterSystemHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady)).Times(1);
    SetParticipantStatus(1, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(2, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);
}

TEST_F(SystemMonitorTest, detect_shuttingdown_after_error)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ControllersCreated);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ShuttingDown))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initializing_after_invalid)
{
    // Test that the monitor recovers from seemingly erroneous state transitions.
    //
    // Due to the distributed nature, it can occur that some participants
    // have not matched yet, while others are already fully connected. This can lead
    // to one participant already starting initalization while the other not having yet
    // connected to the (local) participant, which is seemingly a wrong state transition
    // as the whole system is not idle yet. The SystemMonitor must be able to recover
    // from such erroneous state transitions.

    SetParticipantStatus(1, ParticipantState::ControllersCreated);
    SetParticipantStatus(1, ParticipantState::CommunicationReady);

    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady))
        .Times(1);

    SetParticipantStatus(2, ParticipantState::ControllersCreated);
    SetParticipantStatus(3, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);
}

TEST_F(SystemMonitorTest, detect_coldswapprepare_after_stopped)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);


    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPrepare)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapPrepare);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPrepare);

    SetParticipantStatus(1, ParticipantState::ColdswapReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPrepare);
}

TEST_F(SystemMonitorTest, detect_coldswapready_after_coldswapprepare)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);


    SetParticipantStatus(1, ParticipantState::ColdswapReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPrepare);

    SetParticipantStatus(2, ParticipantState::ColdswapReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPrepare);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapReady)).Times(1);
    SetParticipantStatus(3, ParticipantState::ColdswapReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapReady);
}

TEST_F(SystemMonitorTest, detect_coldswappending_after_coldswapready_due_to_coldswapignore)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPending)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);
}

TEST_F(SystemMonitorTest, detect_coldswappending_after_coldswapready_due_to_coldswapshutdown)
{
    SetAllParticipantStates(ParticipantState::ControllersCreated);
    SetAllParticipantStates(ParticipantState::CommunicationReady);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapPending)).Times(1);
    SetParticipantStatus(1, ParticipantState::ColdswapShutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);
}

TEST_F(SystemMonitorTest, detect_initialized_after_invalid)
{
    // Test that the monitor recovers from seemingly erroneous state transitions.

    SetParticipantStatus(1, ParticipantState::ControllersCreated);
    SetParticipantStatus(1, ParticipantState::CommunicationReady);
    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    SetParticipantStatus(2, ParticipantState::ControllersCreated);
    SetParticipantStatus(2, ParticipantState::CommunicationReady);
    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationReady))
        .Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(3, ParticipantState::ControllersCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::CommunicationReady);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationReady);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);
}

TEST_F(SystemMonitorTest, check_on_partitipant_connected_triggers_callback)
{
    monitor.SetParticipantConnectedHandler([this](const std::string &participantName) {
        callbacks.ParticipantConnectedHandler(participantName);
    });

    const auto participantName = "test participant";
    EXPECT_CALL(callbacks, ParticipantConnectedHandler(participantName));

    EXPECT_FALSE(monitor.IsParticipantConnected(participantName));
    monitor.OnParticipantConnected(participantName);
    EXPECT_TRUE(monitor.IsParticipantConnected(participantName));
}

TEST_F(SystemMonitorTest, check_on_partitipant_disconnected_triggers_callback)
{
    monitor.SetParticipantDisconnectedHandler([this](const std::string &participantName) {
        callbacks.ParticipantDisconnectedHandler(participantName);
    });

    const auto participantName = "test participant";
    EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(participantName));

    EXPECT_FALSE(monitor.IsParticipantConnected(participantName));
    monitor.OnParticipantDisconnected(participantName);
    EXPECT_FALSE(monitor.IsParticipantConnected(participantName));
}

} // anonymous namespace for test
