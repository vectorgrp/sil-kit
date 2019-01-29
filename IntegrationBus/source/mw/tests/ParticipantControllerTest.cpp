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
    SystemCommand command{SystemCommand::Kind::Run};

    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const ParticipantStatus&>()))
        .Times(1);

    controller.ReceiveIbMessage(masterAddr, command);

    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, call_init_handler)
{
    ParticipantCommand command{addr.participant, ParticipantCommand::Kind::Initialize};

    EXPECT_CALL(callbacks, InitHandler(command))
        .Times(1);

    controller.SetInitHandler(bind_method(&callbacks, &Callbacks::InitHandler));
    controller.SetSimulationTask([](auto){});
    controller.RunAsync();
    controller.ReceiveIbMessage(masterAddr, command);
}

TEST_F(ParticipantControllerTest, must_set_simtask_before_calling_run)
{
    EXPECT_THROW(controller.Run(), std::exception);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, calling_run_announces_idle_state)
{
    ParticipantStatus newStatus;
    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const ParticipantStatus&>()))
        .Times(1)
        .WillOnce(SaveArg<1>(&newStatus));

    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);
    controller.RunAsync();

    EXPECT_EQ(controller.State(), ParticipantState::Idle);
    EXPECT_EQ(newStatus.state, ParticipantState::Idle);
}

TEST_F(ParticipantControllerTest, run_async)
{
    auto grantRequest = [&controller = controller, &masterAddr = masterAddr](auto addr, const QuantumRequest& request)
    {
        QuantumGrant grant;
        grant.grantee = addr;
        grant.status = QuantumRequestStatus::Granted;
        grant.now = 0ns;
        grant.duration = request.duration;

        controller.ReceiveIbMessage(masterAddr, grant);
    };

    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const ParticipantStatus&>()))
        .Times(6);

    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const QuantumRequest&>()))
        .Times(2)
        .WillOnce(Invoke(grantRequest));

    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(1);

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));
    auto finalState = controller.RunAsync();

    controller.ReceiveIbMessage(masterAddr, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);
    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}


} // anonymous namespace for test