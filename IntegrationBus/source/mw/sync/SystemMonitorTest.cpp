// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemMonitor.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "SyncDatatypeUtils.hpp"
#include "ib/mw/sync/string_utils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::util;

using ::ib::mw::test::DummyComAdapter;

class SystemMonitorTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(SystemStateHandler, void(SystemState));
        MOCK_METHOD1(ParticipantStateHandler, void(ParticipantState));
        MOCK_METHOD1(ParticipantStatusHandler, void(ParticipantStatus));
    };

protected:
    SystemMonitorTest()
        : testConfig{MakeTestConfig()}
        , monitor{&comAdapter, testConfig.simulationSetup}
    {
        monitor.SetEndpointAddress(addr);
    }

    void RegisterSystemHandler()
    {
        monitor.RegisterSystemStateHandler(bind_method(&callbacks, &Callbacks::SystemStateHandler));
    }
    void RegisterParticipantStateHandler()
    {
        monitor.RegisterParticipantStateHandler(bind_method(&callbacks, &Callbacks::ParticipantStateHandler));
    }
    void RegisterParticipantStatusHandler()
    {
        monitor.RegisterParticipantStatusHandler(bind_method(&callbacks, &Callbacks::ParticipantStatusHandler));
    }

    static auto MakeTestConfig() -> cfg::Config
    {
        cfg::ConfigBuilder builder{"TestConfig"};
        auto&& simulationSetup = builder.SimulationSetup();
        simulationSetup.AddParticipant("P1").WithParticipantId(1).AddParticipantController().WithSyncType(cfg::SyncType::TimeQuantum);
        simulationSetup.AddParticipant("P2").WithParticipantId(2).AddParticipantController().WithSyncType(cfg::SyncType::TimeQuantum);
        simulationSetup.AddParticipant("P3").WithParticipantId(3).AddParticipantController().WithSyncType(cfg::SyncType::TimeQuantum);
        return builder.Build();
    }

    void SetParticipantStatus(ParticipantId participant, ParticipantState state, std::string reason = std::string{})
    {
        auto&& participantCfg = testConfig.simulationSetup.participants.at(participant - 1);
        ParticipantStatus status;
        status.state = state;
        status.participantName = participantCfg.name;
        status.enterReason = reason;

        EndpointAddress from;
        from.participant = participantCfg.id;
        from.endpoint = 1024;
        
        monitor.ReceiveIbMessage(from, status);
    }

    void SetAllParticipantStates(ParticipantState state)
    {
        for (auto&& participantCfg : testConfig.simulationSetup.participants)
        {
            SetParticipantStatus(participantCfg.id, state);
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

    DummyComAdapter comAdapter;
    cfg::Config testConfig;
    SystemMonitor monitor;
    Callbacks callbacks;
};

TEST_F(SystemMonitorTest, init_with_state_invalid)
{
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Invalid);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Invalid);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Invalid);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_idle)
{
    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Idle))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Idle);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Idle);

    SetParticipantStatus(2, ParticipantState::Idle);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Idle);

    SetParticipantStatus(3, ParticipantState::Idle);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Idle);

    EXPECT_EQ(monitor.SystemState(), SystemState::Idle);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_initializing)
{
    SetAllParticipantStates(ParticipantState::Idle);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initializing);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(2, ParticipantState::Initializing);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initializing);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_initialized)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Initialized);

    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_running)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    ASSERT_EQ(monitor.SystemState(), SystemState::Initialized);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    SetParticipantStatus(2, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    SetParticipantStatus(3, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Running);

    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}


TEST_F(SystemMonitorTest, detect_system_pause)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused))
        .Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_multiple_paused_clients)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    ASSERT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Paused))
        .Times(1);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Running))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(2, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(2, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    SetParticipantStatus(1, ParticipantState::Running);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_stopping)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_system_stopped)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopping))
        .Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Stopped))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(1, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(2, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(2, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(3, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    SetParticipantStatus(3, ParticipantState::Stopped);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initializing_after_stopped)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initializing);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initialized_after_stopped)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);
    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_shutdown)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);


    SetParticipantStatus(1, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    SetParticipantStatus(2, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantState(2), ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown))
        .Times(1);
    SetParticipantStatus(3, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantState(3), ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_idle)
{
    SetAllParticipantStates(ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::Idle);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initializing)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_initialized)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_running)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_paused)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Paused);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Paused);
    EXPECT_EQ(monitor.SystemState(), SystemState::Paused);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopping)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    EXPECT_EQ(monitor.SystemState(), SystemState::Running);

    SetParticipantStatus(1, ParticipantState::Stopping);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Stopping);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopping);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_stopped)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_error_from_shuttingdown)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    EXPECT_EQ(monitor.SystemState(), SystemState::Stopped);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.SystemState(), SystemState::ShuttingDown);

    // if the Shutdown callback triggers an error, this can lead to a temporary SystemError state.
    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Error))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    // After the callback, the participant state will be set to Shutdown.
    // The system state will remain in Error until all participants are Shutdown.
    SetParticipantStatus(1, ParticipantState::Shutdown);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Shutdown))
        .Times(1);
    SetAllParticipantStates(ParticipantState::Shutdown);
    EXPECT_EQ(monitor.SystemState(), SystemState::Shutdown);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_initializing_after_error)
{
    SetAllParticipantStates(ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::Idle);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::Initializing);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    EXPECT_EQ(monitor.InvalidTransitionCount(), 0u);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_one_swapping)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetParticipantStatus(3, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_all_swapping)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetParticipantStatus(1, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    SetParticipantStatus(2, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapPending);

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ColdswapDone)).Times(1);
    SetParticipantStatus(3, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);
}

TEST_F(SystemMonitorTest, detect_coldswapdone_after_coldswappending_none_swapping)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
    SetAllParticipantStates(ParticipantState::Initialized);
    SetAllParticipantStates(ParticipantState::Running);
    SetAllParticipantStates(ParticipantState::Stopping);
    SetAllParticipantStates(ParticipantState::Stopped);
    SetAllParticipantStates(ParticipantState::ColdswapPrepare);
    SetAllParticipantStates(ParticipantState::ColdswapReady);
    SetAllParticipantStates(ParticipantState::ColdswapIgnored);
    EXPECT_EQ(monitor.SystemState(), SystemState::ColdswapDone);

    RegisterSystemHandler();

    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing)).Times(1);
    SetParticipantStatus(1, ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(2, ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);
}

TEST_F(SystemMonitorTest, detect_shuttingdown_after_error)
{
    SetAllParticipantStates(ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::Idle);

    SetParticipantStatus(1, ParticipantState::Error);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::Error);
    EXPECT_EQ(monitor.SystemState(), SystemState::Error);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::ShuttingDown))
        .Times(1);

    SetParticipantStatus(1, ParticipantState::ShuttingDown);
    EXPECT_EQ(monitor.ParticipantState(1), ParticipantState::ShuttingDown);
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

    SetParticipantStatus(1, ParticipantState::Idle);
    SetParticipantStatus(1, ParticipantState::Initializing);

    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing))
        .Times(1);

    SetParticipantStatus(2, ParticipantState::Idle);
    SetParticipantStatus(3, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);
}

TEST_F(SystemMonitorTest, detect_coldswapprepare_after_stopped)
{
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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
    SetAllParticipantStates(ParticipantState::Idle);
    SetAllParticipantStates(ParticipantState::Initializing);
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

    SetParticipantStatus(1, ParticipantState::Idle);
    SetParticipantStatus(1, ParticipantState::Initializing);
    SetParticipantStatus(1, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    SetParticipantStatus(2, ParticipantState::Idle);
    SetParticipantStatus(2, ParticipantState::Initializing);
    SetParticipantStatus(2, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Invalid);

    RegisterSystemHandler();
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initializing))
        .Times(1);
    EXPECT_CALL(callbacks, SystemStateHandler(SystemState::Initialized))
        .Times(1);

    SetParticipantStatus(3, ParticipantState::Idle);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initializing);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initializing);

    SetParticipantStatus(3, ParticipantState::Initialized);
    EXPECT_EQ(monitor.SystemState(), SystemState::Initialized);
}


} // anonymous namespace for test
