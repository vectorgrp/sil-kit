// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "JsonConfig.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"
#include "ib/cfg/ConfigBuilder.hpp"

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

class JsonConfigTest : public testing::Test
{
protected:
    JsonConfigTest()
        : builder("TestConfig")
        , simulationSetup{builder.SimulationSetup()}
    {
    }

    void BuildConfigFromJson()
    {
        referenceConfig = builder.Build();
        jsonString = referenceConfig.ToJsonString(); 
        config = ib::cfg::Config::FromJsonString(jsonString);
    }

    auto getClusterParameters() -> ib::sim::fr::ClusterParameters;
    auto getNodeParameters() -> ib::sim::fr::NodeParameters;
    auto getTxBufferConfig() -> ib::sim::fr::TxBufferConfig;

protected:
    ConfigBuilder builder;
    SimulationSetupBuilder& simulationSetup;

    ib::cfg::Config referenceConfig;
    std::string jsonString;
    ib::cfg::Config config;
};

TEST_F(JsonConfigTest, ConfigureLogger)
{
    simulationSetup.AddParticipant("P1").ConfigureLogger().EnableLogFromRemotes().WithFlushLevel(logging::Level::Critical)
        ->AddSink(Sink::Type::Stdout).WithLogLevel(logging::Level::Info)
        ->AddSink(Sink::Type::File).WithLogLevel(logging::Level::Trace).WithLogname("FileLogger");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
    EXPECT_EQ(config.name, "TestConfig");

    auto&& participants = config.simulationSetup.participants;
    ASSERT_EQ(participants.size(), 1u);
    auto&& p1 = participants[0];

    ASSERT_EQ(p1.name, "P1");
    EXPECT_TRUE(p1.logger.logFromRemotes);
    EXPECT_EQ(p1.logger.flush_level, logging::Level::Critical);
    EXPECT_EQ(p1.logger.sinks.size(), 2u);
    EXPECT_EQ(p1.logger.sinks[0].type, Sink::Type::Stdout);
    EXPECT_EQ(p1.logger.sinks[0].level, logging::Level::Info);
    EXPECT_EQ(p1.logger.sinks[1].type, Sink::Type::File);
    EXPECT_EQ(p1.logger.sinks[1].level, logging::Level::Trace);
}

TEST_F(JsonConfigTest, CreateCanNetwork)
{
    simulationSetup.AddParticipant("P1")
        ->AddCan("CAN1").WithLink("CAN_A")
        ->AddCan("CAN2").WithLink("CAN_B");
    simulationSetup.AddParticipant("P2")
        ->AddCan("CAN1").WithLink("CAN_A")
        ->AddCan("CAN2").WithLink("CAN_B");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    EXPECT_EQ(config.name, "TestConfig");

    auto&& participants = config.simulationSetup.participants;
    auto&& links = config.simulationSetup.links;

    ASSERT_EQ(participants.size(), 2u);
    ASSERT_EQ(links.size(), 2u);

    auto&& p1 = participants[0];
    auto&& p2 = participants[1];
    auto&& linkA = links[0];
    auto&& linkB = links[1];

    EXPECT_NE(linkA.id, linkB.id);
    EXPECT_EQ(linkA.name, "CAN_A");
    EXPECT_EQ(linkA.endpoints, std::vector<std::string>({"P1/CAN1", "P2/CAN1"}));
    EXPECT_EQ(linkB.name, "CAN_B");
    EXPECT_EQ(linkB.endpoints, std::vector<std::string>({"P1/CAN2", "P2/CAN2"}));

    ASSERT_EQ(p1.name, "P1");
    EXPECT_EQ(p1.canControllers.size(), 2u);
    EXPECT_EQ(p1.canControllers[0].name, "CAN1");
    EXPECT_EQ(p1.canControllers[0].linkId, linkA.id);
    EXPECT_EQ(p1.canControllers[1].name, "CAN2");
    EXPECT_EQ(p1.canControllers[1].linkId, linkB.id);

    ASSERT_EQ(p2.name, "P2");
    EXPECT_EQ(p2.canControllers.size(), 2u);
    EXPECT_EQ(p2.canControllers[0].name, "CAN1");
    EXPECT_EQ(p2.canControllers[0].linkId, linkA.id);
    EXPECT_EQ(p2.canControllers[1].name, "CAN2");
    EXPECT_EQ(p2.canControllers[1].linkId, linkB.id);
}

TEST_F(JsonConfigTest, CreateLinNetwork)
{
    simulationSetup.AddParticipant("P1")
        ->AddLin("LIN1").WithLink("LIN_A")
        ->AddLin("LIN2").WithLink("LIN_B");
    simulationSetup.AddParticipant("P2")
        ->AddLin("LIN1").WithLink("LIN_A")
        ->AddLin("LIN2").WithLink("LIN_B");

    BuildConfigFromJson();

    auto&& participants = config.simulationSetup.participants;
    auto&& links = config.simulationSetup.links;

    ASSERT_EQ(participants.size(), 2u);
    ASSERT_EQ(links.size(), 2u);

    auto&& p1 = participants[0];
    auto&& p2 = participants[1];
    auto&& linkA = links[0];
    auto&& linkB = links[1];

    EXPECT_NE(linkA.id, linkB.id);
    EXPECT_EQ(linkA.name, "LIN_A");
    EXPECT_EQ(linkA.endpoints, std::vector<std::string>({"P1/LIN1", "P2/LIN1"}));
    EXPECT_EQ(linkB.name, "LIN_B");
    EXPECT_EQ(linkB.endpoints, std::vector<std::string>({"P1/LIN2", "P2/LIN2"}));

    ASSERT_EQ(p1.linControllers.size(), 2u);
    EXPECT_EQ(p1.linControllers[0].name, "LIN1");
    EXPECT_EQ(p1.linControllers[0].linkId, linkA.id);
    EXPECT_EQ(p1.linControllers[1].name, "LIN2");
    EXPECT_EQ(p1.linControllers[1].linkId, linkB.id);

    ASSERT_EQ(p2.linControllers.size(), 2u);
    EXPECT_EQ(p2.linControllers[0].name, "LIN1");
    EXPECT_EQ(p2.linControllers[0].linkId, linkA.id);
    EXPECT_EQ(p2.linControllers[1].name, "LIN2");
    EXPECT_EQ(p2.linControllers[1].linkId, linkB.id);
}

TEST_F(JsonConfigTest, CreateEthernetNetworkSimple)
{
    simulationSetup.AddParticipant("P1")
        ->AddEthernet("ETH1").WithLink("ETH_A")
        ->AddEthernet("ETH2").WithLink("ETH_B");
    simulationSetup.AddParticipant("P2")
        ->AddEthernet("ETH1").WithLink("ETH_A")
        ->AddEthernet("ETH2").WithLink("ETH_B");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& participants = config.simulationSetup.participants;
    auto&& links = config.simulationSetup.links;

    ASSERT_EQ(participants.size(), 2u);
    ASSERT_EQ(links.size(), 2u);

    auto&& p1 = participants[0];
    auto&& p2 = participants[1];
    auto&& linkA = links[0];
    auto&& linkB = links[1];

    EXPECT_NE(linkA.id, linkB.id);
    EXPECT_EQ(linkA.name, "ETH_A");
    EXPECT_EQ(linkA.endpoints, std::vector<std::string>({"P1/ETH1", "P2/ETH1"}));
    EXPECT_EQ(linkB.name, "ETH_B");
    EXPECT_EQ(linkB.endpoints, std::vector<std::string>({"P1/ETH2", "P2/ETH2"}));

    ASSERT_EQ(p1.ethernetControllers.size(), 2u);
    EXPECT_EQ(p1.ethernetControllers[0].name, "ETH1");
    EXPECT_EQ(p1.ethernetControllers[0].linkId, linkA.id);
    EXPECT_EQ(p1.ethernetControllers[1].name, "ETH2");
    EXPECT_EQ(p1.ethernetControllers[1].linkId, linkB.id);

    ASSERT_EQ(p2.ethernetControllers.size(), 2u);
    EXPECT_EQ(p2.ethernetControllers[0].name, "ETH1");
    EXPECT_EQ(p2.ethernetControllers[0].linkId, linkA.id);
    EXPECT_EQ(p2.ethernetControllers[1].name, "ETH2");
    EXPECT_EQ(p2.ethernetControllers[1].linkId, linkB.id);
}

TEST_F(JsonConfigTest, CreateEthernetNetworkWithSwitch)
{
    simulationSetup.AddParticipant("P1")
        ->AddEthernet("ETH1").WithLink("Port0");
    simulationSetup.AddParticipant("P2")
        ->AddEthernet("ETH1").WithLink("Port1");
    simulationSetup.AddSwitch("FrontSwitch")
        ->AddPort("Port0").WithVlanIds({1})
        ->AddPort("Port1").WithVlanIds({2});
    builder.SimulationSetup().AddOrGetLink(Link::Type::Ethernet, "Port0")
        .AddEndpoint("FrontSwitch/Port0");
    builder.SimulationSetup().AddOrGetLink(Link::Type::Ethernet, "Port1")
        .AddEndpoint("FrontSwitch/Port1");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& participants = config.simulationSetup.participants;
    auto&& switches = config.simulationSetup.switches;
    auto&& links = config.simulationSetup.links;

    ASSERT_EQ(participants.size(), 2u);
    ASSERT_EQ(links.size(), 2u);
    ASSERT_EQ(switches.size(), 1u);

    auto&& p1 = participants[0];
    auto&& p2 = participants[1];
    auto&& ethSwitch = switches[0];
    auto&& linkA = links[0];
    auto&& linkB = links[1];

    EXPECT_NE(linkA.id, linkB.id);
    EXPECT_EQ(linkA.name, "Port0");
    EXPECT_EQ(linkA.endpoints, std::vector<std::string>({"P1/ETH1", "FrontSwitch/Port0"}));
    EXPECT_EQ(linkB.name, "Port1");
    EXPECT_EQ(linkB.endpoints, std::vector<std::string>({"P2/ETH1", "FrontSwitch/Port1"}));

    ASSERT_EQ(p1.ethernetControllers.size(), 1u);
    EXPECT_EQ(p1.ethernetControllers[0].name, "ETH1");
    EXPECT_EQ(p1.ethernetControllers[0].linkId, linkA.id);
    EXPECT_EQ(p1.ethernetControllers[0].pcapFile, "");
    EXPECT_EQ(p1.ethernetControllers[0].pcapPipe, "");

    ASSERT_EQ(p2.ethernetControllers.size(), 1u);
    EXPECT_EQ(p2.ethernetControllers[0].name, "ETH1");
    EXPECT_EQ(p2.ethernetControllers[0].linkId, linkB.id);

    
    ASSERT_EQ(ethSwitch.ports.size(), 2u);
    EXPECT_EQ(ethSwitch.name, "FrontSwitch");
    EXPECT_EQ(ethSwitch.ports[0].name, "Port0");
    EXPECT_EQ(ethSwitch.ports[0].vlanIds, std::vector<uint16_t>({1}));
    EXPECT_EQ(ethSwitch.ports[1].name, "Port1");
    EXPECT_EQ(ethSwitch.ports[1].vlanIds, std::vector<uint16_t>({2}));
}

TEST_F(JsonConfigTest, CreateFlexrayNetwork)
{
    simulationSetup.AddParticipant("P1")
        ->AddFlexray("FR1").WithLink("FR_A").WithNodeParameters(getNodeParameters())
        ->AddFlexray("FR2").WithLink("FR_B").WithNodeParameters(getNodeParameters());
    simulationSetup.AddParticipant("P2")
        ->AddFlexray("FR1").WithLink("FR_A").WithNodeParameters(getNodeParameters())
        ->AddFlexray("FR2").WithLink("FR_B").WithNodeParameters(getNodeParameters());

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& participants = config.simulationSetup.participants;
    auto&& links = config.simulationSetup.links;

    ASSERT_EQ(participants.size(), 2u);
    ASSERT_EQ(links.size(), 2u);

    auto&& p1 = participants[0];
    auto&& p2 = participants[1];
    auto&& linkA = links[0];
    auto&& linkB = links[1];

    EXPECT_NE(linkA.id, linkB.id);
    EXPECT_EQ(linkA.name, "FR_A");
    EXPECT_EQ(linkA.endpoints, std::vector<std::string>({"P1/FR1", "P2/FR1"}));
    EXPECT_EQ(linkB.name, "FR_B");
    EXPECT_EQ(linkB.endpoints, std::vector<std::string>({"P1/FR2", "P2/FR2"}));

    ASSERT_EQ(p1.flexrayControllers.size(), 2u);
    EXPECT_EQ(p1.flexrayControllers[0].name, "FR1");
    EXPECT_EQ(p1.flexrayControllers[0].linkId, linkA.id);
    EXPECT_EQ(p1.flexrayControllers[1].name, "FR2");
    EXPECT_EQ(p1.flexrayControllers[1].linkId, linkB.id);

    ASSERT_EQ(p2.flexrayControllers.size(), 2u);
    EXPECT_EQ(p2.flexrayControllers[0].name, "FR1");
    EXPECT_EQ(p2.flexrayControllers[0].linkId, linkA.id);
    EXPECT_EQ(p2.flexrayControllers[1].name, "FR2");
    EXPECT_EQ(p2.flexrayControllers[1].linkId, linkB.id);
}

auto JsonConfigTest::getClusterParameters() -> ib::sim::fr::ClusterParameters
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

auto JsonConfigTest::getNodeParameters() -> ib::sim::fr::NodeParameters
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

auto JsonConfigTest::getTxBufferConfig() -> ib::sim::fr::TxBufferConfig
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

TEST_F(JsonConfigTest, CreateFlexrayParametersNetwork)
{
    auto&& txBufferConfigs = std::vector<ib::sim::fr::TxBufferConfig>{
        getTxBufferConfig(),
        getTxBufferConfig(),
        getTxBufferConfig()
    };

    simulationSetup.AddParticipant("P1")->AddFlexray("FR1")
        .WithLink("FR_A")
        .WithClusterParameters(getClusterParameters())
        .WithNodeParameters(getNodeParameters())
        .WithTxBufferConfigs(txBufferConfigs);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
}

TEST_F(JsonConfigTest, CreateGenericMessageConfig)
{
    const std::string msgName{"FancyRosMessage"};
    const std::string msgDefinition{"file://./ros/messages/FancyRosMessage.msg"};

    simulationSetup.AddParticipant("Publisher")
        .AddGenericPublisher(msgName)
            .WithProtocolType(GenericPort::ProtocolType::ROS)
            .WithDefinitionUri(msgDefinition);
    simulationSetup.AddParticipant("Subscriber")
        .AddGenericSubscriber(msgName);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    ASSERT_EQ(config.simulationSetup.links.size(), 1u);
    auto&& rosLink = config.simulationSetup.links.at(0);

    EXPECT_EQ(rosLink.name, msgName);
    EXPECT_EQ(rosLink.type, Link::Type::GenericMessage);

    auto&& participants = config.simulationSetup.participants;

    EXPECT_EQ(participants.at(0).genericPublishers.at(0).name, msgName);
    EXPECT_EQ(participants.at(1).genericSubscribers.at(0).name, msgName);

    EXPECT_EQ(participants.at(0).genericPublishers.at(0).protocolType, GenericPort::ProtocolType::ROS);
    EXPECT_EQ(participants.at(1).genericSubscribers.at(0).protocolType, GenericPort::ProtocolType::ROS);

    EXPECT_EQ(participants.at(0).genericPublishers.at(0).definitionUri, msgDefinition);
    EXPECT_EQ(participants.at(1).genericSubscribers.at(0).definitionUri, msgDefinition);
}

TEST_F(JsonConfigTest, create_fastrtps_config_default)
{
    builder.ConfigureFastRtps();

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& fastRtps = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastRtps.discoveryType, FastRtps::DiscoveryType::Local);
    EXPECT_EQ(fastRtps.unicastLocators.size(), 0u);
    EXPECT_EQ(fastRtps.configFileName, std::string{});
    EXPECT_EQ(fastRtps.historyDepth, -1);
}

TEST_F(JsonConfigTest, create_fastrtps_history_depth)
{
    builder.ConfigureFastRtps()
        .WithHistoryDepth(8);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& fastRtps = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastRtps.historyDepth, 8);
}

TEST_F(JsonConfigTest, create_fastrtps_config_unicast)
{
    builder.ConfigureFastRtps()
        .WithDiscoveryType(FastRtps::DiscoveryType::Unicast)
        .AddUnicastLocator("Participant1", "192.168.0.1")
        .AddUnicastLocator("Participant2", "192.168.0.2");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& fastRtps = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastRtps.discoveryType, FastRtps::DiscoveryType::Unicast);
    EXPECT_EQ(fastRtps.unicastLocators.size(), 2u);
    EXPECT_EQ(fastRtps.unicastLocators["Participant1"], std::string{"192.168.0.1"});
    EXPECT_EQ(fastRtps.unicastLocators["Participant2"], std::string{"192.168.0.2"});
    EXPECT_EQ(fastRtps.configFileName, std::string{});
}

TEST_F(JsonConfigTest, create_fastrtps_config_multicast)
{
    builder.ConfigureFastRtps()
        .WithDiscoveryType(FastRtps::DiscoveryType::Multicast);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& fastRtps = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastRtps.discoveryType, FastRtps::DiscoveryType::Multicast);
    EXPECT_EQ(fastRtps.unicastLocators.size(), 0u);
    EXPECT_EQ(fastRtps.configFileName, std::string{});
}

TEST_F(JsonConfigTest, create_fastrtps_config_with_configfile)
{
    builder.ConfigureFastRtps()
        .WithDiscoveryType(FastRtps::DiscoveryType::ConfigFile)
        .WithConfigFileName("MyMagicFastRTPSsettings.xml");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& fastRtps = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastRtps.discoveryType, FastRtps::DiscoveryType::ConfigFile);
    EXPECT_EQ(fastRtps.unicastLocators.size(), 0u);
    EXPECT_EQ(fastRtps.configFileName, std::string{"MyMagicFastRTPSsettings.xml"});
}

TEST_F(JsonConfigTest, default_middlware_is_fastrtps)
{
    BuildConfigFromJson();
    EXPECT_EQ(config.middlewareConfig.activeMiddleware, Middleware::NotConfigured);
}

TEST_F(JsonConfigTest, select_fastrtps_as_middleware)
{
    builder.WithActiveMiddleware(ib::cfg::Middleware::FastRTPS);

    BuildConfigFromJson();
    EXPECT_EQ(config.middlewareConfig.activeMiddleware, ib::cfg::Middleware::FastRTPS);
}

TEST_F(JsonConfigTest, select_vasio_as_middleware)
{
    builder.WithActiveMiddleware(ib::cfg::Middleware::VAsio);

    BuildConfigFromJson();
    EXPECT_EQ(config.middlewareConfig.activeMiddleware, ib::cfg::Middleware::VAsio);
}

TEST_F(JsonConfigTest, configure_vasio_registry)
{
    builder.ConfigureVAsio()
        .ConfigureRegistry().WithHostname("NotLocalhost").WithPort(1701)
        .ConfigureLogger().EnableLogFromRemotes().WithFlushLevel(logging::Level::Critical)
        ->AddSink(Sink::Type::Stdout).WithLogLevel(logging::Level::Info);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
}

TEST_F(JsonConfigTest, configure_timesync_with_strict_sync_policy)
{
    simulationSetup
        .ConfigureTimeSync()
          .WithTickPeriod(10ms)
          .WithStrictSyncPolicy();

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& timeSync = config.simulationSetup.timeSync;
    EXPECT_EQ(timeSync.syncPolicy, TimeSync::SyncPolicy::Strict);
    EXPECT_EQ(timeSync.tickPeriod, 10ms);
}

TEST_F(JsonConfigTest, configure_timesync_with_loose_sync_policy)
{
    simulationSetup
        .ConfigureTimeSync()
            .WithTickPeriod(10ms)
            .WithLooseSyncPolicy();

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& timeSync = config.simulationSetup.timeSync;
    EXPECT_EQ(timeSync.syncPolicy, TimeSync::SyncPolicy::Loose);
    EXPECT_EQ(timeSync.tickPeriod, 10ms);
}

TEST_F(JsonConfigTest, default_timesync_policy_is_loose)
{
    simulationSetup.ConfigureTimeSync();

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& timeSync = config.simulationSetup.timeSync;
    EXPECT_EQ(timeSync.syncPolicy, TimeSync::SyncPolicy::Loose);
}

TEST_F(JsonConfigTest, configure_participant_as_syncmaster)
{
    simulationSetup.AddParticipant("SyncMaster")
        .AsSyncMaster();

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& participants = config.simulationSetup.participants;
    ASSERT_EQ(participants.size(), 1u);

    EXPECT_TRUE(participants[0].isSyncMaster);
}

TEST_F(JsonConfigTest, configure_participant_controller)
{
    simulationSetup.AddParticipant("P1").AddParticipantController()
        .WithSyncType(SyncType::DiscreteTime)
        .WithExecTimeLimitSoft(10ms)
        .WithExecTimeLimitHard(20ms);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& controller = config.simulationSetup.participants[0].participantController;
    ASSERT_TRUE(controller.has_value());
    EXPECT_EQ(controller->syncType, SyncType::DiscreteTime);
    EXPECT_EQ(controller->execTimeLimitSoft, 10ms);
    EXPECT_EQ(controller->execTimeLimitHard, 20ms);
}

TEST_F(JsonConfigTest, configure_participant_controller_without_exec_time_limits)
{
    simulationSetup.AddParticipant("P1").AddParticipantController()
        .WithSyncType(SyncType::DiscreteTime);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& controller = config.simulationSetup.participants[0].participantController;
    ASSERT_TRUE(controller.has_value());
    EXPECT_EQ(controller->syncType, SyncType::DiscreteTime);
    EXPECT_EQ(controller->execTimeLimitSoft, std::chrono::milliseconds::max());
    EXPECT_EQ(controller->execTimeLimitHard, std::chrono::milliseconds::max());
}

TEST_F(JsonConfigTest, configure_participant_without_participant_controller)
{
    simulationSetup.AddParticipant("P1");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    auto&& controller = config.simulationSetup.participants[0].participantController;
    EXPECT_FALSE(controller.has_value());
}

// Test the configuration of the extension search path hints
TEST_F(JsonConfigTest, configure_extension_search_path_hints)
{
    builder.ConfigureExtensions()
        .AddSearchPath("Extension/Search/Path/1")
        .AddSearchPath("Extension/Search/Path/2");

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
}

// Test if the default configuration is not serialized
TEST_F(JsonConfigTest, configure_extension_default)
{
    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);

    EXPECT_TRUE(jsonString.find("ExtensionConfig") == std::string::npos);
}

TEST_F(JsonConfigTest, configure_participant_add_tracesink)
{
    //NB: VS2015 does not like constructing of TraceSink{}  with initializer_list
    //    in a vector declaration. Replace makeTraceSink with aggregate init when
    //    we drop VS2015 support.
    auto makeTraceSink = [](auto type, auto name, auto path) {
        ib::cfg::TraceSink sink;
        sink.type = type;
        sink.name = name;
        sink.outputPath = path;
        return sink;
    };

    std::vector<ib::cfg::TraceSink> testTable({
        makeTraceSink(TraceSink::Type::PcapFile, "EthSink", "some/path/EthTraceOputput.pcap"),
        makeTraceSink(TraceSink::Type::Mdf4File, "SinkForCan", "other path/CAN1.mdf4"),
    });
     
    auto&& participant = simulationSetup.AddParticipant("P1");


    for (const auto& sinkArgs : testTable)
    {
        participant.AddTraceSink(sinkArgs.name)
            .WithOutputPath(sinkArgs.outputPath)
            .WithType(sinkArgs.type);
    }


    // create tracers on all supported controller types
    auto&& ethCtrl = participant.AddEthernet("Eth1");
    ethCtrl.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& canCtrl = participant.AddCan("CanCtrl");
    canCtrl.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& linCtrl = participant.AddLin("LinCtrl");
    linCtrl.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& frCtrl = participant.AddFlexray("FlexrayCtrl").WithNodeParameters(getNodeParameters());
    frCtrl.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& doutPort = participant.AddDigitalOut("DigitalOutFoo");
    doutPort.WithUnit("Foo").WithInitValue(false);
    doutPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& dinport = participant.AddDigitalIn("Digi In");
    dinport.WithUnit("Digital Inport Unit").WithInitValue(false);

    auto&& ainPort = participant.AddAnalogIn("AnalogIn");
    ainPort.WithUnit("Foo").WithInitValue(13.37);
    ainPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& aoutPort = participant.AddAnalogOut("AnalogOut");
    aoutPort.WithUnit("Analog Out Unit").WithInitValue(0.1234);

    auto&& patPort = participant.AddPatternIn("PatternIn");
    patPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");
    patPort.WithUnit("Pattern Unit").WithInitValue({'c','a','f','e'});

    auto&& patoutPort = participant.AddPatternOut("Pattern Out");
    patoutPort.WithUnit("Pattern Unit").WithInitValue({'b','e','e','f'});

    auto&& pwmPort = participant.AddPwmOut("PWM OUT");
    pwmPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");
    pwmPort.WithUnit("PWM Unit").WithInitValue({1.0, 0.5});

    auto&& pwmPort2 = participant.AddPwmIn("PwmPort2 IN");
    pwmPort2.WithUnit("some other unit").WithInitValue({3.0, 4.5});


    auto&& genPort = participant.AddGenericPublisher("GenericPublisher");
    genPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    auto&& subPort = participant.AddGenericSubscriber("A Subscriber");
    subPort.WithTraceSink("EthSink").WithTraceSink("SinkForCan");

    BuildConfigFromJson();
    //NB: the following does not hold when adding multiple controllers,
    //    because endpoint IDs are not serialized:
    //          EXPECT_EQ(config, referenceConfig);
    const auto& pold = referenceConfig.simulationSetup.participants[0];
    const auto& pnew = config.simulationSetup.participants[0];
    EXPECT_EQ(
        pnew.traceSinks,
        pold.traceSinks
    );

    const auto& cfgSinks = config.simulationSetup.participants[0].traceSinks;
    ASSERT_EQ(cfgSinks.size(), testTable.size());


    // verify the ser/des of all service controllers. we have to ignore the endpointIDs when comparing.
    auto checkFields = [](const auto& octrl, const auto& nctrl)
    {
        EXPECT_EQ(octrl.name, nctrl.name);
        EXPECT_EQ(octrl.useTraceSinks, nctrl.useTraceSinks);
    };


    checkFields(pnew.genericPublishers.at(0), pold.genericPublishers.at(0));

    checkFields(pnew.ethernetControllers.at(0), pold.ethernetControllers.at(0));
    checkFields(pnew.canControllers.at(0), pold.canControllers.at(0));
    checkFields(pnew.flexrayControllers.at(0), pold.flexrayControllers.at(0));
    checkFields(pnew.linControllers.at(0), pold.linControllers.at(0));

    auto checkPort = [&checkFields](const auto& oports, const auto& nports)
    {
        for (const auto& oldport : oports)
        {
            const auto& oldPortName = oldport.name;
            bool hasMatch = false;
            for (const auto& newport : nports)
            {
                if (newport.name == oldPortName)
                {
                    hasMatch = true;
                    checkFields(oldport, newport);
                }
            }
            ASSERT_TRUE(hasMatch) << "The ports might be serialized in different order, but they have to be present!";
        }
    };

    checkPort(pnew.analogIoPorts, pold.analogIoPorts);
    checkPort(pnew.digitalIoPorts, pold.digitalIoPorts);
    checkPort(pnew.patternPorts, pold.patternPorts);
    checkPort(pnew.pwmPorts, pold.pwmPorts);

    for (auto i = 0u; i < testTable.size(); i++)
    {
        ASSERT_EQ(cfgSinks.at(i).name, testTable.at(i).name);
        ASSERT_EQ(cfgSinks.at(i).outputPath, testTable.at(i).outputPath);
        ASSERT_EQ(cfgSinks.at(i).type, testTable.at(i).type);
    }
}

TEST_F(JsonConfigTest, configure_participant_add_networksimulator)
{
    simulationSetup.AddParticipant("P1")
        .AddNetworkSimulator("NetSimOne")
        .WithLinks({"Link1", "Link2"})
        .WithSwitches({"NetSwitch0", "NetSwitch1"})
        ;

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
}

TEST_F(JsonConfigTest, configure_participant_add_networksimulator_legacy)
{
    auto legacyConfig = ib::cfg::Config::FromJsonString(
        R"(
{
    "ConfigName": "TestConfig",
    "SimulationSetup": {
        "Participants": [
            {
                "Name" : "P1",
                "NetworkSimulators": ["NetSimOne"]
            }
        ],
        "NetworkSimulators" : [
            {
                "Name" : "NetSimOne",
                "SimulatedLinks": [ "Link1", "Link2"],
                "SimulatedSwitches": ["NetSwitch0", "NetSwitch1"]
            }
        ]
    }
})");
    

    simulationSetup.AddParticipant("P1")
        .AddNetworkSimulator("NetSimOne")
        .WithLinks({"Link1", "Link2"})
        .WithSwitches({"NetSwitch0", "NetSwitch1"})
        ;
    referenceConfig = builder.Build();

    ASSERT_EQ(referenceConfig, legacyConfig);
}

TEST_F(JsonConfigTest, configure_trace_source)
{
    auto&& p1 = simulationSetup.AddParticipant("P1");
    p1.AddTraceSource("Source1")
        .WithInputPath("TestPath")
        .WithType(ib::cfg::TraceSource::Type::Mdf4File);

    auto& p2 = simulationSetup.AddParticipant("P2");
    p2.AddTraceSource("DifferentSource")
        .WithInputPath("AnotherFile")
        .WithType(ib::cfg::TraceSource::Type::PcapFile);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
}


TEST_F(JsonConfigTest, configure_replayconfig_direction)
{
    auto&& participant = simulationSetup.AddParticipant("P1");
    participant
        .AddCan("CAN1")
        .WithReplay("Source1")
        .WithDirection(ib::cfg::Replay::Direction::Send);
    participant
        .AddGenericPublisher("Pub1")
        .WithReplay("Source1")
        .WithDirection(ib::cfg::Replay::Direction::Receive);
    participant
        .AddPatternOut("Pat1")
        .WithReplay("Source1")
        .WithDirection(ib::cfg::Replay::Direction::Both);

    BuildConfigFromJson();

    const auto& pold = referenceConfig.simulationSetup.participants.at(0);
    const auto& pnew = config.simulationSetup.participants.at(0);

    EXPECT_EQ(pold.canControllers.at(0).replay, pnew.canControllers.at(0).replay);
    EXPECT_EQ(pold.genericPublishers.at(0).replay, pnew.genericPublishers.at(0).replay);
    EXPECT_EQ(pold.patternPorts.at(0).replay, pnew.patternPorts.at(0).replay);
}

TEST_F(JsonConfigTest, configure_controllers_with_replay)
{

    auto&& participant = simulationSetup.AddParticipant("P1");
    participant.AddTraceSource("Source1")
        .WithInputPath("TestPath")
        .WithType(ib::cfg::TraceSource::Type::Mdf4File);

    // create tracers on all supported controller types
    participant.AddEthernet("Eth1").WithReplay("Source1");
    participant.AddCan("CanCtrl").WithReplay("Source1");
    participant.AddLin("LinCtrl").WithReplay("Source1");

    participant.AddFlexray("FlexrayCtrl")
        .WithNodeParameters(getNodeParameters())
        .WithReplay("Source1");

    participant.AddDigitalOut("DigitalOutFoo").WithReplay("Source1");
    participant.AddDigitalIn("Digi In").WithReplay("Source1");

    participant.AddAnalogIn("AnalogIn").WithReplay("Source1");
    participant.AddAnalogOut("AnalogOut").WithReplay("Source1");

    participant.AddPatternIn("PatternIn").WithReplay("Source1");
    participant.AddPatternOut("Pattern Out").WithReplay("Source1");


    participant.AddPwmOut("PWM OUT").WithReplay("Source1");
    participant.AddPwmIn("PwmPort2 IN").WithReplay("Source1");

    participant.AddGenericPublisher("GenericPublisher").WithReplay("Source1");
    participant.AddGenericSubscriber("A Subscriber").WithReplay("Source1");

    BuildConfigFromJson();
    // NB we compare the replay data directly, services have divergent link Ids which makes comparing at participant-level impossible.
    const auto& pold = referenceConfig.simulationSetup.participants.at(0);
    const auto& pnew = config.simulationSetup.participants.at(0);
    auto compareReplay = [](const auto& lhs, const auto& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;
        for (auto i = 0u; i < lhs.size(); i++)
        {
            if (!(lhs.at(i).replay == rhs.at(i).replay))
                return false;
        }
        return true;
    };

    EXPECT_TRUE(compareReplay(pold.canControllers, pnew.canControllers));
    EXPECT_TRUE(compareReplay(pold.linControllers, pnew.linControllers));
    EXPECT_TRUE(compareReplay(pold.ethernetControllers, pnew.ethernetControllers));
    EXPECT_TRUE(compareReplay(pold.flexrayControllers, pnew.flexrayControllers));
    EXPECT_TRUE(compareReplay(pold.genericPublishers, pnew.genericPublishers));
    EXPECT_TRUE(compareReplay(pold.genericSubscribers, pnew.genericSubscribers));
    EXPECT_TRUE(compareReplay(pold.digitalIoPorts, pnew.digitalIoPorts));
    EXPECT_TRUE(compareReplay(pold.analogIoPorts, pnew.analogIoPorts));
    EXPECT_TRUE(compareReplay(pold.patternPorts, pnew.patternPorts));
    EXPECT_TRUE(compareReplay(pold.pwmPorts, pnew.pwmPorts));
}

TEST_F(JsonConfigTest, configure_registry_connect_attempts)
{
    ib::cfg::Config defaultCfg{};
    ASSERT_EQ(defaultCfg.middlewareConfig.vasio.registry.connectAttempts, 1);

    auto parsedConfig = ib::cfg::Config::FromJsonString(
        R"({
    "MiddlewareConfig": {
        "VAsio": {
            "Registry": {
                "ConnectAttempts": 12345
            }
        }
    }
})"
    );
    ASSERT_EQ(parsedConfig.middlewareConfig.vasio.registry.connectAttempts, 12345);

}
} // anonymous namespace
