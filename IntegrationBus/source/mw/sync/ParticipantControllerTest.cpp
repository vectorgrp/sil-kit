// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "SyncDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::util;

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const QuantumRequest& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const ParticipantStatus& msg));
};

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
    {
        MakeConfig(cfg::SyncType::TimeQuantum);
    }

    void MakeConfig(cfg::SyncType syncType)
    {
        cfg::ConfigBuilder builder{"TestConfig"};
        auto&& sim = builder.SimulationSetup();

        sim.ConfigureTimeSync().WithTickPeriod(1ms);
        sim.AddParticipant("SUT").WithSyncType(syncType);
        sim.AddParticipant("P2").WithSyncType(syncType);

        auto config = builder.Build();
        simulationSetup = std::move(config.simulationSetup);
    }


protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{1, 1024};
    EndpointAddress addrP2{2, 1024};
    EndpointAddress masterAddr{3, 1027};

    MockComAdapter comAdapter;
    Callbacks callbacks;
    cfg::SimulationSetup simulationSetup;
};

// Factory method to create a ParticipantStatus matcher that checks the state field
auto AParticipantStatusWithState(ParticipantState expected)
{
    return MatcherCast<const ParticipantStatus&>(Field(&ParticipantStatus::state, expected));
};

TEST_F(ParticipantControllerTest, report_commands_as_error_before_run_was_called)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);

    EXPECT_CALL(comAdapter, SendIbMessage(addr, A<const ParticipantStatus&>()))
        .Times(1);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, call_init_handler)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
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
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(masterAddr, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(masterAddr, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);
}

TEST_F(ParticipantControllerTest, dont_switch_to_stopped_if_stop_handler_reported_an_error)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(masterAddr, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(masterAddr, runCommand);

    controller.SetStopHandler([&controller = controller] { controller.ReportError("StopHandlerFailed!!"); });

    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Error))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(masterAddr, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, must_set_simtask_before_calling_run)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    EXPECT_THROW(controller.Run(), std::exception);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, calling_run_announces_idle_state)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);

    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Idle)))
        .Times(1);
    controller.RunAsync();

    EXPECT_EQ(controller.State(), ParticipantState::Idle);
}


TEST_F(ParticipantControllerTest, run_async)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
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
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Cmd::Initialize --> Initializing --> Initialized
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Initializing))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Initialized))).Times(1);
    controller.ReceiveIbMessage(masterAddr, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);

    // Cmd::Run --> Running --> Call SimTask()
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Running))).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);

    // Cmd::Stop --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // Cmd::Shutdown --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}


TEST_F(ParticipantControllerTest, refreshstatus_must_not_modify_other_fields)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);

    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Idle)))
        .Times(2);
    controller.RunAsync();

    auto oldStatus = controller.Status();
    std::this_thread::sleep_for(1s);

    controller.RefreshStatus();
    auto newStatus = controller.Status();

    EXPECT_TRUE(newStatus.enterTime < newStatus.refreshTime);
    EXPECT_TRUE(oldStatus.refreshTime < newStatus.refreshTime);

    // Ensure that all other fields are unchanged, i.e., the new status is the
    // same as the old one except for the new refreshTime.
    auto expectedStatus = oldStatus;
    expectedStatus.refreshTime = newStatus.refreshTime;
    EXPECT_EQ(expectedStatus, newStatus);
}


TEST_F(ParticipantControllerTest, run_async_with_synctype_distributedtimequantum)
{
    MakeConfig(cfg::SyncType::DistributedTimeQuantum);
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetEndpointAddress(addr);
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Cmd::Initialize --> Initializing --> Initialized
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Initializing))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Initialized))).Times(1);
    controller.ReceiveIbMessage(masterAddr, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);

    // Cmd::Run --> Running --> Call SimTask()
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Running))).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(2);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);
    NextSimTask nextTask;
    nextTask.timePoint = 0ms;
    nextTask.duration = 1ms;
    controller.ReceiveIbMessage(addrP2, nextTask);
    nextTask.timePoint = 1ms;
    nextTask.duration = 1ms;
    controller.ReceiveIbMessage(addrP2, nextTask);

    // Cmd::Stop --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // Cmd::Shutdown --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr, AParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ReceiveIbMessage(masterAddr, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}


} // anonymous namespace for test
