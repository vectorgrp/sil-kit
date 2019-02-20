// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "ParticipantController.hpp"

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
using namespace ib::mw::test;
using namespace ib::util;

class ParticipantControllerTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(InitHandler, void(ParticipantCommand));
        MOCK_METHOD0(StopHandler, void());
        MOCK_METHOD0(ShutdownHandler, void());
        MOCK_METHOD1(SimTask, void(std::chrono::nanoseconds));
    };

protected:
    ParticipantControllerTest()
        : controller(&comAdapter, MakeParticipantConfig(), cfg::TimeSync{})
    {
        controller.SetEndpointAddress(addr);
    }

    static auto MakeParticipantConfig() -> cfg::Participant
    {
        cfg::Participant cfg;
        cfg.syncType = cfg::SyncType::TimeQuantum;
        return cfg;
    }

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{19, 13};
    EndpointAddress masterAddr{1024, 0};

    MockComAdapter comAdapter;
    ParticipantController controller;
    Callbacks callbacks;
};

// Factory method to create a ParticipantStatus matcher that checks the state field
auto ParticipantStatusWithState = [](ParticipantState expected)
{
    return MatcherCast<const ParticipantStatus&>(Field(&ParticipantStatus::state, expected));
};


TEST_F(ParticipantControllerTest, report_error_on_RunAsync_with_strict_sync)
{
    cfg::TimeSync timesyncConfig;
    timesyncConfig.syncPolicy = cfg::TimeSync::SyncPolicy::Strict;

    controller = ParticipantController(&comAdapter, MakeParticipantConfig(), timesyncConfig);
    controller.SetEndpointAddress(addr);

    auto finalState = controller.RunAsync();

    EXPECT_TRUE(finalState.valid());
    EXPECT_EQ(finalState.get(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, report_commands_as_error_before_run_was_called)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const ParticipantStatus&>()))
        .Times(1);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, call_init_handler)
{
    controller.SetInitHandler(bind_method(&callbacks, &Callbacks::InitHandler));
    controller.SetSimulationTask([](auto){});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    EXPECT_CALL(callbacks, InitHandler(initCommand))
        .Times(1);
    controller.ReceiveIbMessage(masterAddr, initCommand);
}

TEST_F(ParticipantControllerTest, call_stop_handler)
{
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(masterAddr, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Stopped))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(masterAddr, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);
}

TEST_F(ParticipantControllerTest, dont_switch_to_stopped_if_stop_handler_reported_an_error)
{
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(masterAddr, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    controller.SetStopHandler([&controller = controller] { controller.ReportError("StopHandlerFailed!!"); });

    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Error))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(masterAddr, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, must_set_simtask_before_calling_run)
{
    EXPECT_THROW(controller.Run(), std::exception);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, calling_run_announces_idle_state)
{
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);

    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Idle)))
        .Times(1);
    controller.RunAsync();

    EXPECT_EQ(controller.State(), ParticipantState::Idle);
}


TEST_F(ParticipantControllerTest, run_async)
{
    auto grantRequest = [&controller = controller, &masterAddr = masterAddr](auto addr, const QuantumRequest& request)
    {
        QuantumGrant grant;
        grant.grantee = addr;
        grant.status = QuantumRequestStatus::Granted;
        grant.now = request.now;
        grant.duration = request.duration;

        controller.ReceiveIbMessage(masterAddr, grant);
    };

    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const QuantumRequest&>()))
        .Times(2)
        .WillOnce(Invoke(grantRequest));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Cmd::Initialize --> Initializing --> Initialized
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Initializing))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Initialized))).Times(1);
    controller.ReceiveIbMessage(masterAddr, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);

    // Cmd::Run --> Running --> Call SimTask()
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Running))).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);

    // Cmd::Stop --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // Cmd::Shutdown --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, ParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}


} // anonymous namespace for test