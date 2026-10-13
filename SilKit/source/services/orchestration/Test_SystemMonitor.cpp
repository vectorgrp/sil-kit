// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/orchestration/SystemMonitor.hpp"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "util/functional.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "core/mock/participant/MockParticipant.hpp"
#include "services/orchestration/SyncDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class Test_SystemMonitor : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(SystemStateHandler, void(SystemState));
        MOCK_METHOD1(ParticipantStatusHandler, void(ParticipantStatus));

        MOCK_METHOD(void, ParticipantConnectedHandler, (const ParticipantConnectionInformation&), (const));
        MOCK_METHOD(void, ParticipantDisconnectedHandler, (const ParticipantConnectionInformation&), (const));
    };

protected:
    Test_SystemMonitor()
        : monitor{&participant}
        , monitorFrom{&participant}
    {
        syncParticipantNames = {"P1", "P2", "P3"};
        monitor.UpdateRequiredParticipantNames(syncParticipantNames);
        monitor.SetServiceDescriptor(addr);

        /* ON_CALL(participant.logger,
                Log(testing::An<::SilKit::Services::Logging::Level>(),
                    testing::An<::SilKit::Services::Logging::Topic>(), testing::An<const std::string&>()))
            .WillByDefault([](SilKit::Services::Logging::Level level, SilKit::Services::Logging::Topic,
                              const std::string& message) {
            std::ostringstream ss;
            ss << "[" << to_string(level) << "] " << message << '\n';
            std::cout << ss.str() << std::flush;
        });*/
    }

    auto AddSystemStateHandler() -> HandlerId
    {
        return monitor.AddSystemStateHandler(bind_method(&callbacks, &Callbacks::SystemStateHandler));
    }

    void RemoveSystemStateHandler(HandlerId handlerId)
    {
        monitor.RemoveSystemStateHandler(handlerId);
    }

    auto AddParticipantStatusHandler() -> HandlerId
    {
        return monitor.AddParticipantStatusHandler(bind_method(&callbacks, &Callbacks::ParticipantStatusHandler));
    }

    void RemoveParticipantStatusHandler(HandlerId handlerId)
    {
        monitor.RemoveParticipantStatusHandler(handlerId);
    }

    void SetParticipantStatus(ParticipantId participantId, ParticipantState state, std::string reason = std::string{})
    {
        uint64_t id = participantId - 1;
        ParticipantStatus status;
        status.state = state;
        status.participantName = syncParticipantNames.at(static_cast<size_t>(id));
        status.enterReason = reason;

        ServiceDescriptor from{"P1", "N1", "C2", 1024};

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
    ServiceDescriptor addr{"P1", "N1", "C1", 1025};

    std::vector<std::string> syncParticipantNames;

    DummyParticipant participant;
    SystemMonitor monitor;
    SystemMonitor monitorFrom;
    Callbacks callbacks;
};

TEST_F(Test_SystemMonitor, init_with_state_invalid)
{
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);
    EXPECT_THROW(monitor.ParticipantStatus("P1"), SilKitError);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(Test_SystemMonitor, detect_system_controllers_created)
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

TEST_F(Test_SystemMonitor, detect_system_communication_initializing)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);

    AddSystemStateHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing)).Times(1);
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

TEST_F(Test_SystemMonitor, detect_system_communication_initialized)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);

    AddSystemStateHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitialized)).Times(1);
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

TEST_F(Test_SystemMonitor, detect_system_readyToRun)
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

TEST_F(Test_SystemMonitor, detect_system_running)
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

TEST_F(Test_SystemMonitor, detect_system_pause)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused)).Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running)).Times(1);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(Test_SystemMonitor, detect_multiple_paused_clients)
{
    SetAllParticipantStates(ParticipantState::ServicesCreated);
    SetAllParticipantStates(ParticipantState::CommunicationInitializing);
    SetAllParticipantStates(ParticipantState::CommunicationInitialized);
    SetAllParticipantStates(ParticipantState::ReadyToRun);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused)).Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running)).Times(1);

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

TEST_F(Test_SystemMonitor, detect_system_stopped)
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

TEST_F(Test_SystemMonitor, detect_controllers_com_initialized_after_stopped)
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

TEST_F(Test_SystemMonitor, detect_shuttingdown)
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

TEST_F(Test_SystemMonitor, detect_shutdown)
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

TEST_F(Test_SystemMonitor, detect_error_from_controllers_created)
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

TEST_F(Test_SystemMonitor, detect_error_from_initializing)
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

TEST_F(Test_SystemMonitor, detect_error_from_initialized)
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

TEST_F(Test_SystemMonitor, detect_error_from_running)
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

TEST_F(Test_SystemMonitor, detect_error_from_paused)
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

TEST_F(Test_SystemMonitor, detect_error_from_stopping)
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

TEST_F(Test_SystemMonitor, detect_error_from_stopped)
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

TEST_F(Test_SystemMonitor, detect_error_from_shuttingdown)
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

TEST_F(Test_SystemMonitor, detect_shuttingdown_after_error)
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

/*! The system state must stay Invalid until every required participant has reported, and must then
 *  follow the least advanced one.
 *
 *  Test that the monitor recovers from seemingly erroneous state transitions.
 *
 *  Due to the distributed nature, it can occur that some participants
 *  have not matched yet, while others are already fully connected. This can lead
 *  to one participant already starting initalization while the other not having yet
 *  connected to the (local) participant, which is seemingly a wrong state transition
 *  as the whole system is not idle yet. The SystemMonitor must be able to recover
 *  from such erroneous state transitions.
 *
 *  Was DISABLED_detect_initializing_after_invalid: the VIB-807 state machine rework (2022) disabled
 *  it with 'TODO why would this be an error? (CommunicationReady used to be initializing)', and that
 *  TODO was later dropped by "fix remove todos (#382)". The TODO was right - the old
 *  CommunicationReady had been mapped onto CommunicationInitializing here but onto
 *  CommunicationInitialized everywhere else, which left the test expecting the system state to jump
 *  to the *most advanced* participant. The scenario is worth covering; only the expectations were
 *  wrong, and they have been corrected below.
 */
TEST_F(Test_SystemMonitor, detect_system_state_once_all_participants_reported)
{
    // P1 is already initializing while P2 and P3 have not reported at all yet.
    SetParticipantStatus(1, ParticipantState::ServicesCreated);
    SetParticipantStatus(1, ParticipantState::CommunicationInitializing);

    // As long as a required participant is unaccounted for, there is no system state.
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    AddSystemStateHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ServicesCreated)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing)).Times(1);

    SetParticipantStatus(2, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    // With P3 the picture is complete. P2 and P3 are the laggards, so the system is ServicesCreated -
    // it does not jump ahead to where P1 already is.
    SetParticipantStatus(3, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    // Once the laggards catch up, the system state follows.
    SetParticipantStatus(2, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(3, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);
}

/*! The system state must track the single lagging participant all the way up the startup ladder.
 *
 *  Same distributed-startup situation as above, but taken to the extreme: two participants run all
 *  the way to ReadyToRun while the third has not reported at all. Every step the laggard takes must
 *  move the system state with it, and the system state must never run ahead of it.
 *
 *  Was DISABLED_detect_initialized_after_invalid, disabled by the VIB-807 state machine rework (2022)
 *  with 'TODO clarify the purpose of this test' (later dropped by "fix remove todos (#382)"). Its
 *  EXPECT_EQ assertions were in fact correct; what made it fail was a missing expectation for the
 *  ServicesCreated notification, which gmock reported as an unexpected call. The walk is now carried
 *  through to ReadyToRun so that the whole ladder is covered.
 */
TEST_F(Test_SystemMonitor, detect_system_state_follows_lagging_participant)
{
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
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ServicesCreated)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitializing)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::CommunicationInitialized)).Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ReadyToRun)).Times(1);

    SetParticipantStatus(3, ParticipantState::ServicesCreated);
    EXPECT_EQ(monitor.SystemState(), SystemState::ServicesCreated);

    SetParticipantStatus(3, ParticipantState::CommunicationInitializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitializing);

    SetParticipantStatus(3, ParticipantState::CommunicationInitialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::CommunicationInitialized);

    SetParticipantStatus(3, ParticipantState::ReadyToRun);
    EXPECT_EQ(monitor.SystemState(), SystemState::ReadyToRun);
}

TEST_F(Test_SystemMonitor, check_on_partitipant_connected_triggers_callback)
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

TEST_F(Test_SystemMonitor, check_on_partitipant_disconnected_triggers_callback)
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

TEST_F(Test_SystemMonitor, add_and_remove_system_state_and_participant_status_handlers)
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

// ================================================================================================
//  Participants running at different speeds
//
//  The tests above always advance P1, P2 and P3 by one state at a time, in ascending order. Without
//  virtual time synchronization there is no back pressure between participants, so under load one
//  participant can run several states ahead of another. The tests below cover that regime.
//
//  All of them are single-threaded and deterministic - no threads, no sleeps, no timeouts and no
//  wall-clock reads - so they behave identically on a heavily loaded CI machine.
// ================================================================================================

/*! Intermediate system states must not be skipped.
 *
 *  The SystemState handler is fed from an aggregate that is only sampled when a ParticipantStatus
 *  message arrives, but it is consumed as a sequence of transitions. If P1 races ahead to Stopping
 *  before P2 reports Running at all, the aggregate jumps straight from ReadyToRun to Stopping and
 *  SystemState::Running is never emitted.
 *
 *  This is not only an observability problem: LifecycleService::NewSystemState uses
 *  SystemState::Stopping as the trigger that makes a coordinated participant stop. Unlike the
 *  startup transitions - which are backed by the participant-replies and pending-subscription
 *  barriers - the stop path has no barrier behind it, so a skipped state is a missed trigger.
 */
TEST_F(Test_SystemMonitor, intermediate_system_states_are_not_skipped)
{
    monitor.UpdateRequiredParticipantNames({"P1", "P2"});

    std::vector<SystemState> observedStates;
    monitor.AddSystemStateHandler([&observedStates](SystemState systemState) {
        observedStates.push_back(systemState);
    });

    // Both participants reach ReadyToRun in lock-step.
    for (const auto state : {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
                             ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun})
    {
        SetParticipantStatus(1, state);
        SetParticipantStatus(2, state);
    }
    ASSERT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    // P1 runs ahead: it starts, finishes its work and stops before P2 reports Running.
    SetParticipantStatus(1, ParticipantState::Running);
    SetParticipantStatus(1, ParticipantState::Stopping);
    SetParticipantStatus(2, ParticipantState::Running);

    EXPECT_THAT(observedStates, Contains(SystemState::Running))
        << "both participants were running, but SystemState::Running was never reported";
}

/*! A system state change that happens while a handler is being registered must not be lost.
 *
 *  SystemMonitor::AddSystemStateHandler first invokes the handler with the current system state and
 *  only then adds it to the handler list. A change delivered in between is dispatched to a list that
 *  does not yet contain the new handler, and is lost for good: the aggregate only changes when a
 *  participant status changes, so nothing will re-deliver it.
 *
 *  In production the two statements are separated by the IO worker thread; here the change is
 *  injected from inside the initial invocation, which exercises the very same code path and the very
 *  same lost notification without depending on thread timing. There is no deadlock risk:
 *  AddSystemStateHandler invokes the handler outside the SynchronizedHandlers lock, and the
 *  reentrant InvokeAll takes a recursive_mutex over an empty handler list.
 *
 *  LifecycleService::StartLifecycle registers its handlers from the user thread, so the lost
 *  notification can be a coordinated participant's own startup or stop trigger.
 */
TEST_F(Test_SystemMonitor, system_state_change_during_handler_registration_is_not_lost)
{
    monitor.UpdateRequiredParticipantNames({"P1", "P2"});

    for (const auto state : {ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
                             ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun})
    {
        SetParticipantStatus(1, state);
        SetParticipantStatus(2, state);
    }
    SetParticipantStatus(1, ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::ReadyToRun);

    std::vector<SystemState> observedStates;
    bool isFirstInvocation{true};

    monitor.AddSystemStateHandler([this, &observedStates, &isFirstInvocation](SystemState systemState) {
        observedStates.push_back(systemState);

        if (isFirstInvocation)
        {
            isFirstInvocation = false;
            // The system state changes to Running while AddSystemStateHandler sits between invoking
            // this handler and registering it.
            SetParticipantStatus(2, ParticipantState::Running);
        }
    });

    ASSERT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_THAT(observedStates, Contains(SystemState::Running));
    ASSERT_FALSE(observedStates.empty());
    EXPECT_EQ(observedStates.back(), monitor.SystemState())
        << "the handler's last observed system state must not lag behind the monitor";
}

/*! ParticipantStatus() must not hand out a reference to storage that keeps changing.
 *
 *  SystemMonitor::ParticipantStatus returns the pointer produced by
 *  SystemStateTracker::GetParticipantStatus, which is taken after the tracker's mutex has already
 *  been released. The returned reference therefore aliases the live map value.
 *
 *  This test only observes the benign half of the problem - the value changes underneath the caller
 *  while the map node is still alive. The real hazard is worse: SetParticipantStatus assigns the
 *  std::string members while a caller may be reading them, and RemoveParticipant erases the node
 *  outright when a participant disconnects, leaving a dangling reference.
 *
 *  The fix pattern already exists in this directory: LifecycleService::Status() copies under lock
 *  into a 'mutable ParticipantStatus _returnValueForStatus', and SystemStateTracker already offers
 *  the copying overload GetParticipantStatus(name, ParticipantStatus&).
 */
TEST_F(Test_SystemMonitor, participant_status_must_not_alias_mutable_storage)
{
    SetParticipantStatus(1, ParticipantState::ServicesCreated);

    const auto& participantStatus = monitor.ParticipantStatus("P1");
    ASSERT_EQ(participantStatus.state, ParticipantState::ServicesCreated);

    SetParticipantStatus(1, ParticipantState::CommunicationInitializing);

    EXPECT_EQ(participantStatus.state, ParticipantState::ServicesCreated)
        << "the previously returned ParticipantStatus changed when an unrelated update arrived";
}

/*! Invalid participant state transitions must be counted.
 *
 *  SystemMonitor::InvalidTransitionCount() reads _invalidTransitionCount, which is never written
 *  anywhere. The detection moved into SystemStateTracker::ValidateParticipantStateUpdate, which only
 *  logs and deliberately lets the transition through, and the count was never wired back up.
 *
 *  As a result every 'EXPECT_EQ(monitor.InvalidTransitionCount(), 0u)' in this file - there are 27 -
 *  cannot fail. Do not read them as evidence that no invalid transition was detected.
 */
TEST_F(Test_SystemMonitor, invalid_participant_transition_is_counted)
{
    // Invalid -> Running: only ServicesCreated is a valid successor of Invalid.
    SetParticipantStatus(1, ParticipantState::Running);
    ASSERT_EQ(monitor.ParticipantStatus("P1").state, ParticipantState::Running);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 1u);
}

} // anonymous namespace
