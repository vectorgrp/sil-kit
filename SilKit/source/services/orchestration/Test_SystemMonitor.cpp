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
#include "SystemMonitor.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "functional.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "MockParticipant.hpp"
#include "SyncDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class SystemMonitorTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(SystemStateHandler, void(SystemState));
        MOCK_METHOD1(ParticipantStatusHandler, void(ParticipantStatus));

        MOCK_METHOD(void, ParticipantConnectedHandler, (const ParticipantConnectionInformation &), (const));
        MOCK_METHOD(void, ParticipantDisconnectedHandler, (const ParticipantConnectionInformation &), (const));
    };

protected:
    SystemMonitorTest()
        : monitor{&participant}
        , monitorFrom{&participant}
    {
        syncParticipantNames = {"P1", "P2", "P3"};
        monitor.UpdateRequiredParticipantNames(syncParticipantNames);
        monitor.SetServiceDescriptor(addr);
    }

    auto AddSystemStateHandler() -> HandlerId
    {
        return monitor.AddSystemStateHandler(bind_method(&callbacks, &Callbacks::SystemStateHandler));
    }

    void RemoveSystemStateHandler(HandlerId handlerId) { monitor.RemoveSystemStateHandler(handlerId); }

    auto AddParticipantStatusHandler() -> HandlerId
    {
        return monitor.AddParticipantStatusHandler(bind_method(&callbacks, &Callbacks::ParticipantStatusHandler));
    }

    void RemoveParticipantStatusHandler(HandlerId handlerId) { monitor.RemoveParticipantStatusHandler(handlerId); }

    void SetParticipantStatus(ParticipantId participantId, ParticipantState state, std::string reason = std::string{})
    {
        uint64_t id = participantId - 1;
        ParticipantStatus status;
        status.state = state;
        status.participantName = syncParticipantNames.at(static_cast<size_t>(id));
        status.enterReason = reason;

        ServiceDescriptor from{ "P1", "N1", "C2" , 1024};

        monitorFrom.SetServiceDescriptor(from);

        monitor.ReceiveMsg(&monitorFrom, status);
    }

    void SetAllParticipantStates(ParticipantState state)
    {
        for (size_t i = 0; i < syncParticipantNames.size(); i++)
        {
            SetParticipantStatus(i + 1, state);
        }
        EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
    }

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    ServiceDescriptor addr{ "P1", "N1", "C1", 1025};

    std::vector<std::string> syncParticipantNames;

    DummyParticipant participant;
    SystemMonitor monitor;
    SystemMonitor monitorFrom;
    Callbacks callbacks;
};

TEST_F(SystemMonitorTest, init_with_state_invalid)
{
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);
    EXPECT_THROW(monitor.ParticipantStatus("P1"), SilKitError);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_controllers_created)
{
    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ServicesCreated)).Times(1);

    SetParticipantStatus(1, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ServicesCreated);

    SetParticipantStatus(2, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::ServicesCreated);

    SetParticipantStatus(3, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::ServicesCreated);

    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_communication_initializing)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);

    AddSystemStateHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing))
        .Times(1);
    SetParticipantStatus(1, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(2, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(3, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_communication_initialized)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);

    AddSystemStateHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitialized))
        .Times(1);
    SetParticipantStatus(1, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);

    SetParticipantStatus(2, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);

    SetParticipantStatus(3, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_readyToRun)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ReadyToRun)).Times(1);

    SetParticipantStatus(1, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    SetParticipantStatus(2, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    SetParticipantStatus(3, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_running)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    ASSERT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running)).Times(1);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    SetParticipantStatus(2, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    SetParticipantStatus(3, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_pause)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused)).Times(1);

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
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused)).Times(1);

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

TEST_F(SystemMonitorTest, DISABLED_detect_system_stopping)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping)).Times(1);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_stopped)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopped)).Times(1);

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

TEST_F(SystemMonitorTest, DISABLED_detect_reinitializing_after_stopped)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    AddSystemStateHandler();
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_controllers_com_initialized_after_stopped)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ReadyToRun)).Times(1);

    SetParticipantStatus(1, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    SetParticipantStatus(2, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P2").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    SetParticipantStatus(3, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ShuttingDown)).Times(1);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shutdown)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
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

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown)).Times(1);
    SetParticipantStatus(3, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P3").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_controllers_created)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initializing)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initialized)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_running)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_paused)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopping)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopped)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    // if the Shutdown callback triggers an error, this can lead to a temporary SystemError state.
    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error)).Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    // After the callback, the participant state will be set to Shutdown.
    // The system state will remain in Error until all participants are Shutdown.
    SetParticipantStatus(1, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown)).Times(1);
    SetAllParticipantStates(ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, DISABLED_detect_initializing_after_error)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    AddSystemStateHandler();

    SetParticipantStatus(1, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shuttingdown_after_error)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ShuttingDown)).Times(1);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, DISABLED_detect_initializing_after_invalid)
{
    // Test that the monitor recovers from seemingly erroneous state transitions.
    //
    // Due to the distributed nature, it can occur that some participants
    // have not matched yet, while others are already fully connected. This can lead
    // to one participant already starting initalization while the other not having yet
    // connected to the (local) participant, which is seemingly a wrong state transition
    // as the whole system is not idle yet. The SystemMonitor must be able to recover
    // from such erroneous state transitions.

    SetParticipantStatus(1, ParticipantState::ServicesCreated);
    SetParticipantStatus(1, ParticipantState::CommunicationInitializing);

    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing)).Times(1);

    SetParticipantStatus(2, ParticipantState::ServicesCreated);
    SetParticipantStatus(3, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);
}

TEST_F(SystemMonitorTest, DISABLED_detect_initialized_after_invalid)
{
    // Test that the monitor recovers from seemingly erroneous state transitions.

    SetParticipantStatus(1, ParticipantState::ServicesCreated);
    SetParticipantStatus(1, ParticipantState::CommunicationInitializing);
    SetParticipantStatus(1, ParticipantState::CommunicationInitialized);
    SetParticipantStatus(1, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    SetParticipantStatus(2, ParticipantState::ServicesCreated);
    SetParticipantStatus(2, ParticipantState::CommunicationInitializing);
    SetParticipantStatus(2, ParticipantState::CommunicationInitialized);
    SetParticipantStatus(2, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitialized)).Times(1);

    SetParticipantStatus(3, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(3, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);

    SetParticipantStatus(3, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);
}

TEST_F(SystemMonitorTest, check_on_partitipant_connected_triggers_callback)
{
    monitor.SetParticipantConnectedHandler([this](const ParticipantConnectionInformation& participantInformation) {
        callbacks.ParticipantConnectedHandler(participantInformation);
    });
    const ParticipantConnectionInformation pci{"test participant"};
    EXPECT_CALL(callbacks, ParticipantConnectedHandler(pci));

    EXPECT_FALSE(monitor.IsParticipantConnected(pci.participantName));
    monitor.OnParticipantConnected(pci);
    EXPECT_TRUE(monitor.IsParticipantConnected(pci.participantName));
}

TEST_F(SystemMonitorTest, check_on_partitipant_disconnected_triggers_callback)
{
    monitor.SetParticipantDisconnectedHandler([this](const ParticipantConnectionInformation& participantInformation) {
        callbacks.ParticipantDisconnectedHandler(participantInformation);
    });

    const ParticipantConnectionInformation pci{"test participant"};
    EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(pci));

    EXPECT_FALSE(monitor.IsParticipantConnected(pci.participantName));
    monitor.OnParticipantDisconnected(pci);
    EXPECT_FALSE(monitor.IsParticipantConnected(pci.participantName));
}

TEST_F(SystemMonitorTest, add_and_remove_system_state_and_participant_status_handlers)
{
    const auto systemStateHandlerId = AddSystemStateHandler();
    const auto participantStatusHandlerId = AddParticipantStatusHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ServicesCreated)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStatusHandler(testing::AllOf(
                               testing::Field(&ParticipantStatus::participantName, "P1"),
                               testing::Field(&ParticipantStatus::state, ParticipantState::ServicesCreated))))
        .Times(1);
    EXPECT_CALL(callbacks, ParticipantStatusHandler(testing::AllOf(
                               testing::Field(&ParticipantStatus::participantName, "P2"),
                               testing::Field(&ParticipantStatus::state, ParticipantState::ServicesCreated))))
        .Times(1);
    EXPECT_CALL(callbacks, ParticipantStatusHandler(testing::AllOf(
                               testing::Field(&ParticipantStatus::participantName, "P3"),
                               testing::Field(&ParticipantStatus::state, ParticipantState::ServicesCreated))))
        .Times(1);
    SetAllParticipantStates(ParticipantState::ServicesCreated);

    RemoveSystemStateHandler(systemStateHandlerId);
    RemoveParticipantStatusHandler(participantStatusHandlerId);

    EXPECT_CALL(callbacks, SystemStateHandler(testing::_)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStatusHandler(testing::_)).Times(0);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

} // anonymous namespace
