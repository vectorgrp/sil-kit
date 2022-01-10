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
    MOCK_METHOD2(SendIbMessage, void(const IIbServiceEndpoint*, const ParticipantStatus& msg));
};

class MockServiceDescriptor : public IIbServiceEndpoint
{
public:
    ServiceDescriptor serviceDescriptor;
    MockServiceDescriptor(EndpointAddress ea, std::string participantName)
    {
      ServiceDescriptor id = from_endpointAddress(ea);
      id.participantName = participantName;
      SetServiceDescriptor(id);
    }
    void SetServiceDescriptor(const ServiceDescriptor& _serviceDescriptor) override
    {
        serviceDescriptor = _serviceDescriptor;
    }
    auto GetServiceDescriptor() const -> const ServiceDescriptor & override
    {
        return serviceDescriptor;
    }

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
        MakeConfig(cfg::SyncType::DistributedTimeQuantum);
    }

    void MakeConfig(cfg::SyncType syncType)
    {
        cfg::ConfigBuilder builder{"TestConfig"};
        auto&& sim = builder.SimulationSetup();

        sim.ConfigureTimeSync().WithTickPeriod(1ms);
        sim.AddParticipant("SUT").AddParticipantController().WithSyncType(syncType);
        sim.AddParticipant("P2").AddParticipantController().WithSyncType(syncType);

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
    MockServiceDescriptor p2Id{ addrP2, "P2" };
    MockServiceDescriptor masterId{ masterAddr, "Master" };

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
    controller.SetServiceDescriptor(from_endpointAddress(addr));

    EXPECT_CALL(comAdapter, SendIbMessage(&controller, A<const ParticipantStatus&>()))
        .Times(1);

    SystemCommand runCommand{SystemCommand::Kind::Run};

    controller.ReceiveIbMessage(&masterId, runCommand);

    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, call_init_handler)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetInitHandler(bind_method(&callbacks, &Callbacks::InitHandler));
    controller.SetSimulationTask([](auto){});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    EXPECT_CALL(callbacks, InitHandler(initCommand))
        .Times(1);
    controller.ReceiveIbMessage(&masterId, initCommand);
}

TEST_F(ParticipantControllerTest, call_stop_handler)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(&masterId, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(&masterId, runCommand);

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);
}

TEST_F(ParticipantControllerTest, dont_switch_to_stopped_if_stop_handler_reported_an_error)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask([](auto) {});

    controller.RunAsync();

    ParticipantCommand initCommand{addr.participant, ParticipantCommand::Kind::Initialize};
    controller.ReceiveIbMessage(&masterId, initCommand);

    SystemCommand runCommand{SystemCommand::Kind::Run};
    controller.ReceiveIbMessage(&masterId, runCommand);

    controller.SetStopHandler([&controller = controller] { controller.ReportError("StopHandlerFailed!!"); });

    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Error))).Times(1);

    SystemCommand stopCommand{SystemCommand::Kind::Stop};
    controller.ReceiveIbMessage(&masterId, stopCommand);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, must_set_simtask_before_calling_run)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    EXPECT_THROW(controller.Run(), std::exception);
    EXPECT_EQ(controller.State(), ParticipantState::Error);
}

TEST_F(ParticipantControllerTest, calling_run_announces_idle_state)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);

    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle)))
        .Times(1);
    controller.RunAsync();

    EXPECT_EQ(controller.State(), ParticipantState::Idle);
}

// TODO replace time mode by a supported one (distributed time quantum)
// TODO deactivated test for now
TEST_F(ParticipantControllerTest, DISABLED_run_async)
{
    const auto& participant = simulationSetup.participants.at(0);
    ParticipantController controller(&comAdapter, simulationSetup, participant);
    ServiceDescriptor id{};
    id.linkName = "default";
    id.participantName = participant.name;
    id.legacyEpa = addr;
    controller.SetServiceDescriptor(from_endpointAddress(addr));//legacy interface
    controller.SetServiceDescriptor(id);
    //auto grantRequest = [&controller = controller, &masterId = masterId](auto addr, const QuantumRequest& request)
    //{
    //    QuantumGrant grant;
    //    grant.grantee = addr->GetServiceDescriptor();
    //    grant.status = QuantumRequestStatus::Granted;
    //    grant.now = request.now;
    //    grant.duration = request.duration;
    //    
    //    controller.ReceiveIbMessage(&masterId, grant);
    //};
    //
    //EXPECT_CALL(comAdapter, SendIbMessage(&controller, A<const QuantumRequest&>()))
    //    .Times(2)
    //    .WillOnce(Invoke(grantRequest));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Cmd::Initialize --> Initializing --> Initialized
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Initializing))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Initialized))).Times(1);
    controller.ReceiveIbMessage(&masterId, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);

    // Cmd::Run --> Running --> Call SimTask()
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Running))).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(1);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);

    // Cmd::Stop --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // Cmd::Shutdown --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}


TEST_F(ParticipantControllerTest, refreshstatus_must_not_modify_other_fields)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    EXPECT_EQ(controller.State(), ParticipantState::Invalid);

    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle)))
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
    //MakeConfig(cfg::SyncType::DistributedTimeQuantum);
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Cmd::Initialize --> Initializing --> Initialized
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Initializing))).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Initialized))).Times(1);
    controller.ReceiveIbMessage(&masterId, ParticipantCommand{addr.participant, ParticipantCommand::Kind::Initialize});
    EXPECT_EQ(controller.State(), ParticipantState::Initialized);

    // Cmd::Run --> Running --> Call SimTask()
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Running))).Times(1);
    EXPECT_CALL(callbacks, SimTask(_)).Times(2);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Run});
    EXPECT_EQ(controller.State(), ParticipantState::Running);
    NextSimTask nextTask;
    nextTask.timePoint = 0ms;
    nextTask.duration = 1ms;
    controller.ReceiveIbMessage(&p2Id, nextTask);
    nextTask.timePoint = 1ms;
    nextTask.duration = 1ms;
    controller.ReceiveIbMessage(&p2Id, nextTask);

    // Cmd::Stop --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Stop});
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // Cmd::Shutdown --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ReceiveIbMessage(&masterId, SystemCommand{SystemCommand::Kind::Shutdown});
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}

TEST_F(ParticipantControllerTest, force_shutdown)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // Stop() --> Stopping --> Call StopHandler() --> Stopped
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopping))).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Stopped))).Times(1);
    controller.Stop("I quit!");
    EXPECT_EQ(controller.State(), ParticipantState::Stopped);

    // ForceShutdown() --> ShuttingDown --> Call ShutdownHandler() --> Shutdown
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::ShuttingDown))).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Shutdown))).Times(1);
    controller.ForceShutdown("I really, really quit!");
    EXPECT_EQ(controller.State(), ParticipantState::Shutdown);

    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}

TEST_F(ParticipantControllerTest, force_shutdown_is_ignored_if_not_stopped)
{
    ParticipantController controller(&comAdapter, simulationSetup, simulationSetup.participants[0]);
    controller.SetServiceDescriptor(from_endpointAddress(addr));
    controller.SetSimulationTask(bind_method(&callbacks, &Callbacks::SimTask));

    controller.SetStopHandler(bind_method(&callbacks, &Callbacks::StopHandler));
    controller.SetShutdownHandler(bind_method(&callbacks, &Callbacks::ShutdownHandler));

    // Run() --> Idle
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, AParticipantStatusWithState(ParticipantState::Idle))).Times(1);
    auto finalState = controller.RunAsync();

    // ForceShutdown() --> Log::Error --> don't change state, don't call shutdown handlers
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, A<const ParticipantStatus&>())).Times(0);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(0);
    EXPECT_CALL(comAdapter, SendIbMessage(&controller, A<const ParticipantStatus&>())).Times(0);
    controller.ForceShutdown("I really, really quit!");

    // command shall be ignored. State shall be unchanged
    EXPECT_EQ(controller.State(), ParticipantState::Idle);
    ASSERT_EQ(finalState.wait_for(1ms), std::future_status::timeout);
}


} // anonymous namespace for test
