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
    simulationSetup.AddParticipant("L1")
        ->AddLogger(Logger::Type::Remote, logging::Level::warn)
        ->AddLogger(Logger::Type::File, logging::Level::debug)
        .WithFilename("FileLogger");

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& participant = config.simulationSetup.participants[0];

    ASSERT_EQ(participant.logger.size(), 2u);
    auto&& logger1 = participant.logger[0];
    auto&& logger2 = participant.logger[1];

    EXPECT_EQ(logger1.type, Logger::Type::Remote);
    EXPECT_EQ(logger1.level, logging::Level::warn);
    EXPECT_EQ(logger2.type, Logger::Type::File);
    EXPECT_EQ(logger2.level, logging::Level::debug);
}

TEST_F(ConfigBuilderTest, make_eth_controller)
{
    simulationSetup.AddParticipant("P1")
        .AddEthernet("ETH1")
            .WithEndpointId(17)
            .WithLink("LAN0")
            .WithMacAddress("A1:A2:A3:A4:A5:A6:");

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

    simulationSetup.AddParticipant("NetworkSimulator")
        .AddNetworkSimulator("BusSim")
        .WithLinks(links)
        .WithSwitches(switches);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& participant = config.simulationSetup.participants[0];

    std::vector<std::string> simulators{"BusSim"};
    EXPECT_EQ(participant.networkSimulators, simulators);

    
    ASSERT_EQ(config.simulationSetup.networkSimulators.size(), 1u);
    auto&& netsim = config.simulationSetup.networkSimulators[0];
    EXPECT_EQ(netsim.name, "BusSim");
    EXPECT_EQ(netsim.simulatedLinks, std::vector<std::string>(links));
    EXPECT_EQ(netsim.simulatedSwitches, std::vector<std::string>(switches));
}

TEST_F(ConfigBuilderTest, make_pwm_port)
{
    simulationSetup.AddParticipant("P1")
        .AddPwmOut("PWM_O")
            .WithInitValue({3.4, 0.4})
            .WithUnit("kHz")
            .WithLink("PWM1");
    simulationSetup.AddParticipant("P2")
        .AddPwmIn("PWM_I")
        .WithLink("PWM1");

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 2u);
    auto&& p1 = config.simulationSetup.participants[0];
    auto&& p2 = config.simulationSetup.participants[1];

    ASSERT_EQ(p1.pwmPorts.size(), 1u);
    auto&& pwmOut = p1.pwmPorts[0];

    ASSERT_EQ(p2.pwmPorts.size(), 1u);
    auto&& pwmIn = p2.pwmPorts[0];

    ASSERT_EQ(config.simulationSetup.links.size(), 1u);
    auto&& link = config.simulationSetup.links[0];

    EXPECT_EQ(link.name, "PWM1");
    EXPECT_EQ(link.endpoints.size(), 2u);
    EXPECT_EQ(link.type, Link::Type::PwmIo);
    EXPECT_NE(std::find(link.endpoints.begin(), link.endpoints.end(), "P1/PWM_O"),
        link.endpoints.end());
    EXPECT_NE(std::find(link.endpoints.begin(), link.endpoints.end(), "P2/PWM_I"),
        link.endpoints.end());


    EXPECT_EQ(pwmOut.name, "PWM_O");
    EXPECT_EQ(pwmOut.linkId, link.id);
    EXPECT_EQ(pwmOut.endpointId, 1);
    EXPECT_EQ(pwmOut.direction, PortDirection::Out);

    EXPECT_EQ(pwmIn.name, "PWM_I");
    EXPECT_EQ(pwmIn.linkId, link.id);
    EXPECT_EQ(pwmIn.endpointId, 2);
    EXPECT_EQ(pwmIn.direction, PortDirection::In);
}

TEST_F(ConfigBuilderTest, configure_participant_sync_type)
{
    simulationSetup.AddParticipant("P1").WithSyncType(SyncType::TimeQuantum);

    auto config = configBuilder.Build();

    ASSERT_EQ(config.simulationSetup.participants.size(), 1u);
    auto&& p1 = config.simulationSetup.participants[0];
    EXPECT_EQ(p1.syncType, SyncType::TimeQuantum);

    auto json = config.ToJsonString();
    auto jsonConfig = Config::FromJsonString(json);
    EXPECT_EQ(jsonConfig.simulationSetup.participants[0].syncType, SyncType::TimeQuantum);
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

TEST_F(ConfigBuilderTest, configure_fast_rtps_default)
{
    auto config = configBuilder.Build();
    auto&& fastrtpsConfig = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastrtpsConfig.discoveryType, FastRtps::DiscoveryType::Local);
    EXPECT_EQ(fastrtpsConfig.configFileName, std::string{});
    EXPECT_EQ(fastrtpsConfig.unicastLocators.size(), 0u);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_unicast)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Unicast)
        .AddUnicastLocator("participant1", "192.168.0.1")
        .AddUnicastLocator("participant2", "192.168.0.2");

    auto config = configBuilder.Build();

    auto&& fastrtpsConfig = config.middlewareConfig.fastRtps;

    EXPECT_EQ(fastrtpsConfig.discoveryType, FastRtps::DiscoveryType::Unicast);
    EXPECT_EQ(fastrtpsConfig.configFileName, std::string{});
    EXPECT_EQ(fastrtpsConfig.unicastLocators.size(), 2u);
    EXPECT_EQ(fastrtpsConfig.unicastLocators["participant1"], "192.168.0.1");
    EXPECT_EQ(fastrtpsConfig.unicastLocators["participant2"], "192.168.0.2");
}

TEST_F(ConfigBuilderTest, configure_fastrtps_unicast_without_locators_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Unicast);

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_unicast_with_configfile_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Unicast)
        .WithConfigFileName("UnicastConfig.xml");

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}


TEST_F(ConfigBuilderTest, configure_fastrtps_multicast)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Multicast);

    auto config = configBuilder.Build();

    auto&& fastrtpsConfig = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastrtpsConfig.discoveryType, FastRtps::DiscoveryType::Multicast);
    EXPECT_EQ(fastrtpsConfig.configFileName, std::string{});
    EXPECT_EQ(fastrtpsConfig.unicastLocators.size(), 0u);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_multicast_with_unicastlocators_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Multicast)
        .AddUnicastLocator("participant1", "192.168.0.1");

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_multicast_with_configfile_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::Multicast)
        .WithConfigFileName("MulticastConfig.xml");

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_configfile)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::ConfigFile)
        .WithConfigFileName("MyFastrtpsConfig.xml");

    auto config = configBuilder.Build();

    auto&& fastrtpsConfig = config.middlewareConfig.fastRtps;
    EXPECT_EQ(fastrtpsConfig.discoveryType, FastRtps::DiscoveryType::ConfigFile);
    EXPECT_EQ(fastrtpsConfig.configFileName, std::string{"MyFastrtpsConfig.xml"});
    EXPECT_EQ(fastrtpsConfig.unicastLocators.size(), 0u);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_configfile_without_configfilename_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::ConfigFile);

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}

TEST_F(ConfigBuilderTest, configure_fastrtps_configfile_with_unicast_locators_must_fail)
{
    configBuilder.ConfigureFastRtps()
        .WithDiscoveryType(ib::cfg::FastRtps::DiscoveryType::ConfigFile)
        .AddUnicastLocator("participant1", "192.168.0.1");

    EXPECT_THROW(configBuilder.Build(), ib::cfg::Misconfiguration);
}

TEST_F(ConfigBuilderTest, configure_timesync_syncpolicy_by_parameter)
{
    configBuilder.SimulationSetup().ConfigureTimeSync()
        .WithSyncPolicy(ib::cfg::TimeSync::SyncPolicy::Strict);

    EXPECT_EQ(configBuilder.Build().simulationSetup.timeSync.syncPolicy, ib::cfg::TimeSync::SyncPolicy::Strict);
}

} // anonymous namespace
