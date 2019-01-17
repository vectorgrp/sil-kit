// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SyncMaster.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/sync/SyncDatatypes.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"

#include "SyncDatatypeUtils.hpp"
#include "MockComAdapter.hpp"

// use an anonymous namespace to avoid name clashes
namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;

using namespace testing;

class MockMonitor : public ISystemMonitor
{
public:
    using ISystemMonitor::SystemStateHandlerT;
    using ISystemMonitor::ParticipantStateHandlerT;
    using ISystemMonitor::ParticipantStatusHandlerT;
public:
    void RegisterSystemStateHandler(SystemStateHandlerT handler) override {
        systemStateHandler = std::move(handler);
    }
    void RegisterParticipantStateHandler(ParticipantStateHandlerT handler) override
    {
        participantStateHandler = std::move(handler);
    }
    void RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler) override
    {
        participantStatusHandler = std::move(handler);
    }
    auto SystemState() const -> sync::SystemState override
    {
        return systemState;
    }
    auto ParticipantState(ParticipantId participantId) const -> sync::ParticipantState override
    {
        return participantStatus.at(participantId).state;
    }
    auto ParticipantStatus(ParticipantId participantId) const -> const sync::ParticipantStatus& override
    {
        return participantStatus.at(participantId);
    }

    SystemStateHandlerT systemStateHandler;
    ParticipantStateHandlerT participantStateHandler;
    ParticipantStatusHandlerT participantStatusHandler;
    sync::SystemState systemState;
    std::map<ParticipantId, sync::ParticipantStatus> participantStatus;
};


class SyncMasterTest : public testing::Test
{
protected:
    static auto From(const cfg::Participant& participant) -> mw::EndpointAddress
    {
        return EndpointAddress{participant.id, 0};
    }

protected:
    ib::mw::test::MockComAdapter comAdapter;
    MockMonitor mockMonitor;
};



TEST_F(SyncMasterTest, discrete_time_only)
{
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .ConfigureTimeSync().WithTickPeriod(10ms);
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::DiscreteTime);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::DiscreteTime);
    auto ibConfig = testConfig.Build();

    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{0ns}))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{10ms}))
        .Times(1);

    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);

    TickDone tickDone;
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[1]), tickDone);
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[2]), tickDone);
}

TEST_F(SyncMasterTest, start_running_after_systemstateInvalid)
{
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .ConfigureTimeSync().WithTickPeriod(10ms);
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::DiscreteTime);
    auto ibConfig = testConfig.Build();

    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{0ns}))
        .Times(1);

    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Invalid);
    mockMonitor.systemStateHandler(sync::SystemState::Running);
}

TEST_F(SyncMasterTest, dont_generate_ticks_while_paused)
{
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .ConfigureTimeSync().WithTickPeriod(10ms);
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::DiscreteTime);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::DiscreteTime);
    auto ibConfig = testConfig.Build();

    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{0ns}))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{10ms}))
        .Times(0);


    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Idle);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    // System::Running will cause the 0ns Tick
    mockMonitor.systemStateHandler(sync::SystemState::Running);
    // System::Paused should stop tick generation...
    mockMonitor.systemStateHandler(sync::SystemState::Paused);
    // ... even if the clients send a tick done
    TickDone tickDone;
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[1]), tickDone);
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[2]), tickDone);
}

TEST_F(SyncMasterTest, continue_tick_generation_after_pause)
{
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .ConfigureTimeSync().WithTickPeriod(10ms);
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::DiscreteTime);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::DiscreteTime);
    auto ibConfig = testConfig.Build();

    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{0ns}))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{10ms}))
        .Times(1);


    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Idle);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    // System::Running will cause the 0ns Tick
    mockMonitor.systemStateHandler(sync::SystemState::Running);
    // System::Paused should stop tick generation...
    mockMonitor.systemStateHandler(sync::SystemState::Paused);
    // ... even if the clients send a tick done
    TickDone tickDone;
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[1]), tickDone);
    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[2]), tickDone);
    // But the ticks should continue after switching back to running
    mockMonitor.systemStateHandler(sync::SystemState::Running);
}

TEST_F(SyncMasterTest, single_quantum_client)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    auto ibConfig = testConfig.Build();

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);

    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};

    QuantumGrant grant;
    grant.grantee = addr_p1;
    grant.now = 0ns;
    grant.duration = 1ms;
    grant.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    grant.now = 1ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    grant.now = 2ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ns, 1ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{1ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{2ms, 1ms});
}

TEST_F(SyncMasterTest, two_quantum_clients)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::TimeQuantum);
    auto ibConfig = testConfig.Build();

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);

    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};
    auto addr_p2 = EndpointAddress{ibConfig.simulationSetup.participants[2].id, 1024};

    QuantumGrant grant;
    grant.now = 0ns;
    grant.duration = 1ms;
    grant.status = QuantumRequestStatus::Granted;

    grant.grantee = addr_p1;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);
    grant.grantee = addr_p2;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    grant.now = 1ms;
    grant.grantee = addr_p1;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);
    grant.grantee = addr_p2;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    grant.now = 2ms;
    grant.grantee = addr_p1;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);
    grant.grantee = addr_p2;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ns, 1ms});
    syncMaster.ReceiveIbMessage(addr_p2, QuantumRequest{0ns, 1ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{1ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p2, QuantumRequest{1ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{2ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p2, QuantumRequest{2ms, 1ms});
}

TEST_F(SyncMasterTest, two_quantum_clients_different_periods)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::TimeQuantum);
    auto ibConfig = testConfig.Build();

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);

    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};
    auto addr_p2 = EndpointAddress{ibConfig.simulationSetup.participants[2].id, 1024};

    QuantumGrant grant_p1;
    grant_p1.grantee = addr_p1;
    grant_p1.now = 0ns;
    grant_p1.duration = 1ms;
    grant_p1.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p1))
        .Times(1);
    grant_p1.now = 1ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p1))
        .Times(1);
    grant_p1.now = 2ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p1))
        .Times(1);
    grant_p1.now = 3ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p1))
        .Times(1);


    QuantumGrant grant_p2;
    grant_p2.grantee = addr_p2;
    grant_p2.now = 0ms;
    grant_p2.duration = 2ms;
    grant_p2.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p2))
        .Times(1);
    grant_p2.now = 2ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant_p2))
        .Times(1);



    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p2, QuantumRequest{0ms, 2ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{1ms, 1ms});
    
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{2ms, 1ms});
    syncMaster.ReceiveIbMessage(addr_p2, QuantumRequest{2ms, 2ms});
    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{3ms, 1ms});

}

TEST_F(SyncMasterTest, mixed_clients)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    simulationSetup
        .AddParticipant("client-1").WithSyncType(cfg::SyncType::DiscreteTime);
    auto ibConfig = testConfig.Build();
    ibConfig.simulationSetup.timeSync.tickPeriod = 1ms;

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));
    
    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};
    auto addr_p2 = EndpointAddress{ibConfig.simulationSetup.participants[2].id, 1024};

    QuantumGrant grant;
    grant.grantee = addr_p1;
    grant.now = 0ns;
    grant.duration = 1ms;
    grant.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);
    grant.now = 1ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);
    grant.now = 2ms;
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);


    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{0ns}))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{1ms}))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), Tick{2ms}))
        .Times(1);



    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);

    TickDone tickDone;

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ns, 1ms});

    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[1]), tickDone);

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{1ms, 1ms});

    syncMaster.ReceiveIbMessage(From(ibConfig.simulationSetup.participants[2]), tickDone);

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{2ms, 1ms});
}


TEST_F(SyncMasterTest, dont_grant_quantum_requests_while_paused)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    auto ibConfig = testConfig.Build();

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    // bring system in Paused state
    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);
    mockMonitor.systemStateHandler(sync::SystemState::Paused);

    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};

    QuantumGrant grant;
    grant.grantee = addr_p1;
    grant.now = 0ns;
    grant.duration = 1ms;
    grant.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(0);

    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ns, 1ms});
}

TEST_F(SyncMasterTest, give_grants_after_pause_ends)
{
    // make a config
    ib::cfg::ConfigBuilder testConfig{"TestConfig"};
    auto&& simulationSetup = testConfig.SimulationSetup();
    simulationSetup
        .AddParticipant("master").AsSyncMaster();
    simulationSetup
        .AddParticipant("client-0").WithSyncType(cfg::SyncType::TimeQuantum);
    auto ibConfig = testConfig.Build();

    // Create the SyncMaster
    SyncMaster syncMaster{&comAdapter, ibConfig, &mockMonitor};
    syncMaster.SetEndpointAddress(From(ibConfig.simulationSetup.participants[0]));

    // bring system in Paused state
    assert(mockMonitor.systemStateHandler);
    mockMonitor.systemStateHandler(sync::SystemState::Initialized);
    mockMonitor.systemStateHandler(sync::SystemState::Running);
    mockMonitor.systemStateHandler(sync::SystemState::Paused);

    auto addr_p1 = EndpointAddress{ibConfig.simulationSetup.participants[1].id, 1024};

    QuantumGrant grant;
    grant.grantee = addr_p1;
    grant.now = 0ns;
    grant.duration = 1ms;
    grant.status = QuantumRequestStatus::Granted;

    EXPECT_CALL(comAdapter, SendIbMessage(syncMaster.EndpointAddress(), grant))
        .Times(1);


    syncMaster.ReceiveIbMessage(addr_p1, QuantumRequest{0ns, 1ms});
    mockMonitor.systemStateHandler(sync::SystemState::Running);
}


} // namespace (anonymous)
