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
        auto jsonString = referenceConfig.ToJsonString();      
        config = ib::cfg::Config::FromJsonString(jsonString);
    }

    auto getClusterParameters() -> ib::sim::fr::ClusterParameters;
    auto getNodeParameters() -> ib::sim::fr::NodeParameters;
    auto getTxBufferConfig() -> ib::sim::fr::TxBufferConfig;

protected:
    ConfigBuilder builder;
    SimulationSetupBuilder& simulationSetup;

    ib::cfg::Config referenceConfig;
    ib::cfg::Config config;
};

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
    auto&& rosLink = config.simulationSetup.links[0];

    EXPECT_EQ(rosLink.name, msgName);
    EXPECT_EQ(rosLink.type, Link::Type::GenericMessage);

    auto&& participants = config.simulationSetup.participants;

    EXPECT_EQ(participants[0].genericPublishers[0].name, msgName);
    EXPECT_EQ(participants[1].genericSubscribers[0].name, msgName);

    EXPECT_EQ(participants[0].genericPublishers[0].protocolType, GenericPort::ProtocolType::ROS);
    EXPECT_EQ(participants[1].genericSubscribers[0].protocolType, GenericPort::ProtocolType::ROS);

    EXPECT_EQ(participants[0].genericPublishers[0].definitionUri, msgDefinition);
    EXPECT_EQ(participants[1].genericSubscribers[0].definitionUri, msgDefinition);
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
    builder.ConfigureVAsio().ConfigureRegistry()
        .WithHostname("NotLocalhost")
        .WithPort(1701);

    BuildConfigFromJson();
    EXPECT_EQ(config, referenceConfig);
    EXPECT_EQ(config.middlewareConfig.vasio.registry.hostname, "NotLocalhost");
    EXPECT_EQ(config.middlewareConfig.vasio.registry.port, 1701);
    EXPECT_EQ(config.middlewareConfig.activeMiddleware, ib::cfg::Middleware::VAsio);
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

} // anonymous namespace
