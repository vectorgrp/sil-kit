// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ConfigBuilder.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib::mw;
using namespace ib::cfg;

class ConfigBuilderTest : public testing::Test
{
protected:
    ConfigBuilderTest()
        : configBuilder("TestBuilder")
        , simulationSetup{configBuilder.SimulationSetup()}
    {
    }

protected:
    ConfigBuilder configBuilder;
    SimulationSetupBuilder& simulationSetup;
};

TEST_F(ConfigBuilderTest, make_exec_controller)
{
    simulationSetup.ConfigureTimeSync().WithTickPeriod(10ms);
    simulationSetup.AddParticipant("ExecControllerDt").AsSyncMaster();


    auto config = configBuilder.Build();
    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);

    auto&& execController = config.simulationSetup.participants[0];
    EXPECT_EQ(execController.name, "ExecControllerDt");
    EXPECT_TRUE(execController.isSyncMaster);
    EXPECT_EQ(config.simulationSetup.timeSync.tickPeriod, 10ms);

}

TEST_F(ConfigBuilderTest, configure_logger_config)
{
    simulationSetup.AddParticipant("L1").ConfigureLogger().EnableLogFromRemotes().WithFlushLevel(logging::Level::Critical)
        ->AddSink(Sink::Type::Remote).WithLogLevel(logging::Level::Warn)
        ->AddSink(Sink::Type::File).WithLogLevel(logging::Level::Debug).WithLogname("FileLogger");

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& participant = config.simulationSetup.participants[0];

    EXPECT_TRUE(participant.logger.logFromRemotes);
    EXPECT_EQ(participant.logger.flush_level, logging::Level::Critical);
    ASSERT_EQ(participant.logger.sinks.size(), 2u);
    auto&& sink1 = participant.logger.sinks[0];
    auto&& sink2 = participant.logger.sinks[1];

    EXPECT_EQ(sink1.type, Sink::Type::Remote);
    EXPECT_EQ(sink1.level, logging::Level::Warn);
    EXPECT_EQ(sink2.type, Sink::Type::File);
    EXPECT_EQ(sink2.level, logging::Level::Debug);
}

TEST_F(ConfigBuilderTest, make_eth_controller)
{
    simulationSetup.AddParticipant("P1")
        .AddEthernet("ETH1")
        .WithEndpointId(17)
        .WithLink("LAN0")
        .WithMacAddress("A1:A2:A3:A4:A5:A6:")
        .WithTraceSink("Eth1Sink");

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& participant = config.simulationSetup.participants[0];

    ASSERT_EQ(participant.ethernetControllers.size(), 1u);
    auto&& controller = participant.ethernetControllers[0];

    ASSERT_EQ(config.simulationSetup.links.size(), 1u);
    auto&& link = config.simulationSetup.links[0];

    EXPECT_EQ(link.name, "LAN0");
    EXPECT_EQ(link.endpoints.size(), 1u);
    EXPECT_EQ(link.type, Link::Type::Ethernet);
    EXPECT_NE(std::find(link.endpoints.begin(), link.endpoints.end(), "P1/ETH1"),
              link.endpoints.end());

    std::array<uint8_t, 6> expectedMac{{0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6}};

    EXPECT_EQ(controller.name, "ETH1");
    EXPECT_EQ(controller.macAddress, expectedMac);
    EXPECT_EQ(controller.linkId, link.id);
    EXPECT_EQ(controller.endpointId, 17);
    EXPECT_EQ(controller.useTraceSinks, std::vector<std::string>{"Eth1Sink"});
}

auto getClusterParameters() -> ib::sim::fr::ClusterParameters
{
    ib::sim::fr::ClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 8;
    clusterParams.gCycleCountMax = 63;
    clusterParams.gdActionPointOffset = 2;
    clusterParams.gdDynamicSlotIdlePhase = 1;
    clusterParams.gdMiniSlot = 5;
    clusterParams.gdMiniSlotActionPointOffset = 2;
    clusterParams.gdStaticSlot = 31;
    clusterParams.gdSymbolWindow = 1;
    clusterParams.gdSymbolWindowActionPointOffset = 1;
    clusterParams.gdTSSTransmitter = 9;
    clusterParams.gdWakeupTxActive = 60;
    clusterParams.gdWakeupTxIdle = 180;
    clusterParams.gListenNoise = 2;
    clusterParams.gMacroPerCycle = 3636;
    clusterParams.gMaxWithoutClockCorrectionFatal = 2;
    clusterParams.gMaxWithoutClockCorrectionPassive = 2;
    clusterParams.gNumberOfMiniSlots = 291;
    clusterParams.gNumberOfStaticSlots = 70;
    clusterParams.gPayloadLengthStatic = 16;
    clusterParams.gSyncFrameIDCountMax = 15;
    return clusterParams;
}

auto getNodeParameters() -> ib::sim::fr::NodeParameters
{
    ib::sim::fr::NodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 1;
    nodeParams.pAllowPassiveToActive = 0;
    nodeParams.pChannels = ib::sim::fr::Channel::AB;
    nodeParams.pClusterDriftDamping = 2;
    nodeParams.pdAcceptedStartupRange = 212;
    nodeParams.pdListenTimeout = 400162;
    nodeParams.pKeySlotId = 0;
    nodeParams.pKeySlotOnlyEnabled = 0;
    nodeParams.pKeySlotUsedForStartup = 0;
    nodeParams.pKeySlotUsedForSync = 0;
    nodeParams.pLatestTx = 249;
    nodeParams.pMacroInitialOffsetA = 3;
    nodeParams.pMacroInitialOffsetB = 3;
    nodeParams.pMicroInitialOffsetA = 6;
    nodeParams.pMicroInitialOffsetB = 6;
    nodeParams.pMicroPerCycle = 200000;
    nodeParams.pOffsetCorrectionOut = 127;
    nodeParams.pOffsetCorrectionStart = 3632;
    nodeParams.pRateCorrectionOut = 81;
    nodeParams.pWakeupChannel = ib::sim::fr::Channel::A;
    nodeParams.pWakeupPattern = 33;
    nodeParams.pdMicrotick = ib::sim::fr::ClockPeriod::T25NS;
    nodeParams.pSamplesPerMicrotick = 2;
    return nodeParams;
}

auto getTxBufferConfig() -> ib::sim::fr::TxBufferConfig
{
    ib::sim::fr::TxBufferConfig txBufferConfig;
    txBufferConfig.channels = ib::sim::fr::Channel::A;
    txBufferConfig.slotId = 0;
    txBufferConfig.offset = 0;
    txBufferConfig.repetition = 0;
    txBufferConfig.hasPayloadPreambleIndicator = false;
    txBufferConfig.headerCrc = 0;
    txBufferConfig.transmissionMode = ib::sim::fr::TransmissionMode::SingleShot;
    return txBufferConfig;
}

TEST_F(ConfigBuilderTest, make_fr_controller)
{
    auto txBufferConfigs = std::vector<ib::sim::fr::TxBufferConfig>{
        getTxBufferConfig(),
        getTxBufferConfig(),
        getTxBufferConfig()
    };

    simulationSetup.AddParticipant("FR").AddFlexray("FR1")
        .WithClusterParameters(getClusterParameters())
        .WithNodeParameters(getNodeParameters())
        .WithTxBufferConfigs(txBufferConfigs);

    auto config = configBuilder.Build();

    auto&& participant = config.simulationSetup.participants[0];
    EXPECT_EQ(participant.flexrayControllers[0].clusterParameters, getClusterParameters());
    EXPECT_EQ(participant.flexrayControllers[0].nodeParameters, getNodeParameters());
    EXPECT_EQ(participant.flexrayControllers[0].txBufferConfigs, txBufferConfigs);
}

TEST_F(ConfigBuilderTest, make_switch)
{
    simulationSetup.AddSwitch("SWITCH")
        ->AddPort("Port0").WithVlanIds({1,2,3}).WithEndpointId(9)
        ->AddPort("Port1").WithVlanIds({3}).WithEndpointId(10);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.switches.size(), 1u);
    auto&& switch_ = config.simulationSetup.switches[0];

    EXPECT_EQ(switch_.name, "SWITCH");
    ASSERT_EQ(switch_.ports.size(), 2u);

    auto&& p0 = switch_.ports[0];
    decltype(p0.vlanIds) p0_vlans = {1,2,3};
    EXPECT_EQ(p0.name, "Port0");
    EXPECT_EQ(p0.endpointId, 9);
    EXPECT_EQ(p0.vlanIds, p0_vlans);

    auto&& p1 = switch_.ports[1];
    decltype(p1.vlanIds) p1_vlans = {3};
    EXPECT_EQ(p1.name, "Port1");
    EXPECT_EQ(p1.endpointId, 10);
    EXPECT_EQ(p1.vlanIds, p1_vlans);
}

TEST_F(ConfigBuilderTest, make_network_simulator)
{
    std::initializer_list<std::string> links{"P0", "P1"};
    std::initializer_list<std::string> switches{"SW1", "SW2"};
    const auto targetSimulator = ib::cfg::NetworkSimulator{
        "BusSim",
        links,
        switches,
        {}, //tracesinks
        {}, //replay
    };

    simulationSetup.AddParticipant("NetworkSimulator")
        .AddNetworkSimulator("BusSim")
        .WithLinks(links)
        .WithSwitches(switches);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& participant = config.simulationSetup.participants[0];

    std::vector<ib::cfg::NetworkSimulator> simulators{targetSimulator};
    EXPECT_EQ(participant.networkSimulators, simulators);

    
    ASSERT_EQ(participant.networkSimulators.size(), 1u);
    auto&& netsim = participant.networkSimulators[0];
    EXPECT_EQ(netsim.name, "BusSim");
    EXPECT_EQ(netsim.simulatedLinks, std::vector<std::string>(links));
    EXPECT_EQ(netsim.simulatedSwitches, std::vector<std::string>(switches));
}

TEST_F(ConfigBuilderTest, configure_participant_sync_type)
{
    simulationSetup.AddParticipant("P1").WithSyncType(SyncType::TimeQuantum);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& p1 = config.simulationSetup.participants[0];
    ASSERT_TRUE(p1.participantController);
    EXPECT_EQ(p1.participantController->syncType, SyncType::TimeQuantum);

    auto json = config.ToJsonString();
    auto jsonConfig = Config::FromJsonString(json);
    ASSERT_TRUE(jsonConfig.simulationSetup.participants[0].participantController.has_value());
    EXPECT_EQ(jsonConfig.simulationSetup.participants[0].participantController->syncType, SyncType::TimeQuantum);
}

TEST_F(ConfigBuilderTest, make_generic_message_config)
{
    const std::string msgName{"FancyRosMessage"};
    const std::string msgDefinition{"file://./ros/messages/FancyRosMessage.msg"};

    simulationSetup.AddParticipant("Publisher")
        .AddGenericPublisher(msgName)
            .WithProtocolType(GenericPort::ProtocolType::ROS)
            .WithDefinitionUri(msgDefinition);
    simulationSetup.AddParticipant("Subscriber")
        .AddGenericSubscriber(msgName);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 2u);
    auto&& pub = config.simulationSetup.participants[0];
    auto&& sub = config.simulationSetup.participants[1];

    ASSERT_EQ(pub.genericPublishers.size(), 1u);
    ASSERT_EQ(pub.genericSubscribers.size(), 0u);
    ASSERT_EQ(sub.genericPublishers.size(), 0u);
    ASSERT_EQ(sub.genericSubscribers.size(), 1u);

    auto&& publisher = pub.genericPublishers[0];
    EXPECT_EQ(publisher.name, msgName);
    EXPECT_EQ(publisher.protocolType, GenericPort::ProtocolType::ROS);
    EXPECT_EQ(publisher.definitionUri, msgDefinition);

    auto&& subscriber = sub.genericSubscribers[0];
    EXPECT_EQ(subscriber.name, msgName);
    
    ASSERT_EQ(config.simulationSetup.links.size(), 1u);
    auto&& rosLink = config.simulationSetup.links[0];

    EXPECT_EQ(rosLink.name, msgName);
    EXPECT_EQ(rosLink.type, Link::Type::GenericMessage);
    EXPECT_EQ(rosLink.endpoints, std::vector<std::string>({"Publisher/FancyRosMessage", "Subscriber/FancyRosMessage"}));
}

TEST_F(ConfigBuilderTest, configure_vasio_registry)
{
    const std::string hostname = "hostname";
    const uint16_t port = 12345;

    configBuilder.ConfigureVAsio().ConfigureRegistry().WithHostname(hostname).WithPort(port)
        .ConfigureLogger().EnableLogFromRemotes().WithFlushLevel(logging::Level::Critical)
        ->AddSink(Sink::Type::Remote).WithLogLevel(logging::Level::Warn)
        ->AddSink(Sink::Type::File).WithLogLevel(logging::Level::Debug).WithLogname("FileLogger");

    auto config = configBuilder.Build();
    auto&& registry = config.middlewareConfig.vasio.registry;

    EXPECT_EQ(registry.hostname, hostname);
    EXPECT_EQ(registry.port, port);

    EXPECT_TRUE(registry.logger.logFromRemotes);
    EXPECT_EQ(registry.logger.flush_level, logging::Level::Critical);

    ASSERT_EQ(registry.logger.sinks.size(), 2u);
    auto&& sink1 = registry.logger.sinks[0];
    auto&& sink2 = registry.logger.sinks[1];

    EXPECT_EQ(sink1.type, Sink::Type::Remote);
    EXPECT_EQ(sink1.level, logging::Level::Warn);
    EXPECT_EQ(sink2.type, Sink::Type::File);
    EXPECT_EQ(sink2.level, logging::Level::Debug);
}

TEST_F(ConfigBuilderTest, configure_timesync_syncpolicy_by_parameter)
{
    configBuilder.SimulationSetup().ConfigureTimeSync()
        .WithSyncPolicy(ib::cfg::TimeSync::SyncPolicy::Strict);

    EXPECT_EQ(configBuilder.Build().simulationSetup.timeSync.syncPolicy, ib::cfg::TimeSync::SyncPolicy::Strict);
}

// Test the generation of the extension configuration
TEST_F(ConfigBuilderTest, configure_extensions)
{
    const std::string extensionSearchPath1 = "Extension/Search/Path/1";
    const std::string extensionSearchPath2 = "Extension/Search/Path/2";

    configBuilder.ConfigureExtensions()
        .AddSearchPath(extensionSearchPath1)
        .AddSearchPath(extensionSearchPath2);

    auto config = configBuilder.Build();

    EXPECT_EQ(config.extensionConfig.searchPathHints.size(), 2);
    EXPECT_EQ(config.extensionConfig.searchPathHints[0], extensionSearchPath1);
    EXPECT_EQ(config.extensionConfig.searchPathHints[1], extensionSearchPath2);
}

static ConfigBuilder DefineBuilder()
{
    ConfigBuilder config_builder("ConfigBuilder on stack");
    auto& simulation_setup = config_builder.SimulationSetup();
    simulation_setup.ConfigureTimeSync().WithStrictSyncPolicy().WithTickPeriod(1ms);

    simulation_setup.AddOrGetLink(Link::Type::GenericMessage, "topic_a")
        .AddEndpoint("publisher/topic_a")
        .AddEndpoint("subscriber/topic_a");

    simulation_setup.AddOrGetLink(Link::Type::GenericMessage, "topic_b")
        .AddEndpoint("publisher/topic_b")
        .AddEndpoint("subscriber/topic_b");

    simulation_setup.AddParticipant("publisher")
        ->WithParticipantId(1)
        ->AddGenericPublisher("topic_a")
        ->AddGenericPublisher("topic_b")
        ->AsSyncMaster()
        ->AddParticipantController()
        ->WithSyncType(SyncType::DiscreteTime);


    simulation_setup.AddParticipant("subscriber")
        ->WithParticipantId(2)
        ->AddGenericSubscriber("topic_a")
        ->AddGenericSubscriber("topic_b")
        ->AddParticipantController()
        ->WithSyncType(SyncType::DiscreteTime);
    return config_builder;
}

TEST_F(ConfigBuilderTest, ensure_configbuilder_is_movable)
{
    auto builder = DefineBuilder();
    auto config = builder.Build();
    auto json = config.ToJsonString();
    EXPECT_TRUE(json.size() > 0);
}

TEST_F(ConfigBuilderTest, configure_tracesources)
{
    auto&& p1 = simulationSetup.AddParticipant("P1");
    p1.AddTraceSource("Source1")
        .WithInputPath("TestPath")
        .WithType(ib::cfg::TraceSource::Type::Mdf4File);
    auto p1config = p1.Build();

    auto& p2 = simulationSetup.AddParticipant("P2");
    p2.AddTraceSource("DifferentSource")
        .WithInputPath("AnotherFile")
        .WithType(ib::cfg::TraceSource::Type::PcapFile);
    auto p2config = p2.Build();

    ASSERT_NE(p1config.traceSources, p2config.traceSources);

    ASSERT_EQ(p1config.traceSources.size(), 1);
    ASSERT_EQ(p2config.traceSources.size(), 1);

    ib::cfg::TraceSource ts1;
    ts1.name = "Source1";
    ts1.inputPath = "TestPath";
    ts1.type = ib::cfg::TraceSource::Type::Mdf4File;

    ASSERT_EQ(ts1, p1config.traceSources.at(0));
    ib::cfg::TraceSource ts2;
    ts2.name = "DifferentSource";
    ts2.inputPath = "AnotherFile";
    ts2.type = ib::cfg::TraceSource::Type::PcapFile;

    ASSERT_EQ(ts2, p2config.traceSources.at(0));
}

} // anonymous namespace
