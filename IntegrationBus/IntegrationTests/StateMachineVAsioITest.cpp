// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class VAsioNetworkITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(InitHandler, void(ib::mw::ParticipantId, ParticipantCommand::Kind));
        MOCK_METHOD0(StopHandler, void());
        MOCK_METHOD0(ShutdownHandler, void());
        MOCK_METHOD1(ParticipantStateHandler, void(ParticipantState));
    };

protected:
    VAsioNetworkITest() = default;

    auto SetTargetState(ParticipantState state)
    {
        _targetState = state;
        _targetStatePromise = std::promise<void>{};
        return _targetStatePromise.get_future();
    }

    void ParticipantStateHandler(ParticipantState state)
    {
        callbacks.ParticipantStateHandler(state);

        if (state == _targetState)
            _targetStatePromise.set_value();
    }

protected:
    ParticipantState _targetState{ParticipantState::Invalid};
    std::promise<void> _targetStatePromise;

    Callbacks callbacks;
};

TEST_F(VAsioNetworkITest, vasio_state_machine)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    // Create a minimal IbConfig
    ib::cfg::ConfigBuilder builder{"TestConfig"};

    builder.SimulationSetup()
        .AddParticipant("TestUnit")
            .AddParticipantController()
                .WithSyncType(ib::cfg::SyncType::DiscreteTime);
    builder.SimulationSetup()
        .AddParticipant("TestController")
            .AsSyncMaster();

    auto ibConfig = builder.Build();

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    // Setup ComAdapter for TestController
    auto comAdapterController = CreateVAsioComAdapterImpl(ibConfig, "TestController");
    comAdapterController->joinIbDomain(domainId);
    auto systemController = comAdapterController->GetSystemController();
    auto monitor = comAdapterController->GetSystemMonitor();

    monitor->RegisterParticipantStateHandler([this](ParticipantState state)
    {
        this->ParticipantStateHandler(state);
    });

    // Setup ComAdapter for Test Unit
    auto comAdapterTestUnit = CreateVAsioComAdapterImpl(ibConfig, "TestUnit");
    comAdapterTestUnit->joinIbDomain(domainId);
    auto participantController = comAdapterTestUnit->GetParticipantController();

    participantController->SetInitHandler([&callbacks = callbacks](ParticipantCommand initCommand) {
        callbacks.InitHandler(initCommand.participant, initCommand.kind);
    });
    participantController->SetSimulationTask([](auto /*now*/, auto /*duration*/) {});

    participantController->SetStopHandler([&callbacks = callbacks]() {
        callbacks.StopHandler();
    });
    participantController->SetShutdownHandler([&callbacks = callbacks]() {
        callbacks.ShutdownHandler();
    });

    auto participantId = ib::cfg::get_by_name(ibConfig.simulationSetup.participants, "TestUnit").id;
    ParticipantCommand initCommand{participantId, ParticipantCommand::Kind::Initialize};

    EXPECT_CALL(callbacks, InitHandler(initCommand.participant, initCommand.kind)).Times(1);
    EXPECT_CALL(callbacks, StopHandler()).Times(1);
    EXPECT_CALL(callbacks, ShutdownHandler()).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Idle)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Initializing)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Initialized)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(1);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);

    // Perform the actual test
    auto stateReached = SetTargetState(ParticipantState::Idle);
    auto finalState = participantController->RunAsync();
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Initialized);
    systemController->Initialize(participantId);
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Running);
    systemController->Run();
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Stopped);
    systemController->Stop();
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    stateReached = SetTargetState(ParticipantState::Shutdown);
    systemController->Shutdown();
    EXPECT_EQ(stateReached.wait_for(5s), std::future_status::ready);

    ASSERT_EQ(finalState.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(finalState.get(), ParticipantState::Shutdown);
}

} // anonymous namespace
