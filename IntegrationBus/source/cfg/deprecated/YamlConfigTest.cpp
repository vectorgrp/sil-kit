// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"
#include "YamlValidator.hpp"

#include "ib/cfg/Config.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
    //!< Yaml config which has almost complete list of config elements.
const auto demoYaml= R"raw(
ConfigName: FlexRayDemoNetSim
ConfigVersion: 0.0.1
Description: Amalgamation of multiple Trace/Replay configs to test YAML Validation
LaunchConfigurations:
- Name: Installation
  ParticipantEnvironments:
  - Arguments: '%INTEGRATIONBUS_CONFIGFILE% Node0 %INTEGRATIONBUS_DOMAINID%'
    Environment: CustomExecutable
    Executable: '%INTEGRATIONBUS_BINPATH%IbDemoFlexray'
    Participant: Node0
    WorkingFolder: .
  - Arguments: '%INTEGRATIONBUS_CONFIGFILE% Node1 %INTEGRATIONBUS_DOMAINID%'
    Environment: CustomExecutable
    Executable: '%INTEGRATIONBUS_BINPATH%IbDemoFlexray'
    Participant: Node1
    WorkingFolder: .
  - Arguments: '%INTEGRATIONBUS_CONFIGFILE% %INTEGRATIONBUS_DOMAINID%'
    Environment: CustomExecutable
    Executable: '%INTEGRATIONBUS_BINPATH%IbSystemMonitor'
    Participant: SystemMonitor
    WorkingFolder: .
  - Arguments: '%INTEGRATIONBUS_CONFIGFILE% %INTEGRATIONBUS_DOMAINID%'
    Environment: CustomExecutable
    Executable: '%INTEGRATIONBUS_BINPATH%IbSystemController'
    Participant: SystemController
    WorkingFolder: .
MiddlewareConfig:
  ActiveMiddleware: VAsio
  VAsio:
    Registry:
      Hostname: NotLocalhost
      Logger:
        Sinks:
        - Type: Remote
      Port: 1337
      ConnectAttempts: 9
    TcpNoDelay: true
    TcpQuickAck: true
    EnableDomainSockets: false
    TcpSendBufferSize: 3456
    TcpReceiveBufferSize: 3456
SimulationSetup:
  Links:
  - Endpoints:
    - Node0/FlexRay1
    - Node1/FlexRay1
    Name: FlexRay-Cluster-1
  - Endpoints:
    - Node0/CAN1
    - Node1/CAN1
    Name: CAN1
  - Endpoints:
    - Node0/GroundTruth
    - Node1/GroundTruth
    Name: GroundTruth
  - Endpoints:
    - Node0/DataPubSubGroundTruth
    - Node1/DataPubSubGroundTruth
    Name: DataPubSubGroundTruth
  - Endpoints:
    - Node0/ETH0
    - Node1/ETH0
    Name: LAN1
  - Endpoints:
    - Node0/LIN1
    - Node1/LIN1
    Name: LIN1
  - Endpoints:
    - Node0/DIO
    - Node1/DIO
    Name: DIO
  - Endpoints:
    - Node0/AIO
    - Node1/AIO
    Name: AIO
  - Endpoints:
    - Node0/PWM
    - Node1/PWM
    Name: PWM
  - Endpoints:
    - Node0/PATTERN
    - Node1/PATTERN
    Name: PATTERN
  Participants:
  - Analog-Out:
    - Name: AIO
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
      unit: V
      value: 7.3
    CanControllers:
    - Name: CAN1
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    Digital-Out:
    - Name: DIO
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
      value: false
    EthernetControllers:
    - MacAddr: F6:04:68:71:AA:C2
      Name: ETH0
    FlexRayControllers:
    - ClusterParameters:
        gColdstartAttempts: 8
        gCycleCountMax: 63
        gListenNoise: 2
        gMacroPerCycle: 3636
        gMaxWithoutClockCorrectionFatal: 2
        gMaxWithoutClockCorrectionPassive: 2
        gNumberOfMiniSlots: 291
        gNumberOfStaticSlots: 70
        gPayloadLengthStatic: 16
        gSyncFrameIDCountMax: 15
        gdActionPointOffset: 2
        gdDynamicSlotIdlePhase: 1
        gdMiniSlot: 5
        gdMiniSlotActionPointOffset: 2
        gdStaticSlot: 31
        gdSymbolWindow: 1
        gdSymbolWindowActionPointOffset: 1
        gdTSSTransmitter: 9
        gdWakeupTxActive: 60
        gdWakeupTxIdle: 180
      Name: FlexRay1
      NodeParameters:
        pAllowHaltDueToClock: 1
        pAllowPassiveToActive: 0
        pChannels: AB
        pClusterDriftDamping: 2
        pKeySlotId: 10
        pKeySlotOnlyEnabled: 0
        pKeySlotUsedForStartup: 1
        pKeySlotUsedForSync: 0
        pLatestTx: 249
        pMacroInitialOffsetA: 3
        pMacroInitialOffsetB: 3
        pMicroInitialOffsetA: 6
        pMicroInitialOffsetB: 6
        pMicroPerCycle: 200000
        pOffsetCorrectionOut: 127
        pOffsetCorrectionStart: 3632
        pRateCorrectionOut: 81
        pSamplesPerMicrotick: 2
        pWakeupChannel: A
        pWakeupPattern: 33
        pdAcceptedStartupRange: 212
        pdListenTimeout: 400162
        pdMicrotick: 25ns
      UseTraceSinks:
      - Sink1
    GenericPublishers:
    - DefinitionUri: file://./sim/groundtruth_topic.msg
      Name: GroundTruth
      Protocol: ROS
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    DataPublishers:
    - Name: DataPubSubGroundTruth
      UseTraceSinks:
      - Sink1
    IsSyncMaster: true
    LinControllers:
    - Name: LIN1
      UseTraceSinks:
      - Sink1
    Logger:
      Sinks:
      - Type: Remote
    Name: Node0
    ParticipantController:
      SyncType: DiscreteTime
    Pattern-Out:
    - Name: PATTERN
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
      value: 626565702d62656570
    Pwm-Out:
    - Name: PWM
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
      duty: 0.4
      freq:
        unit: Hz
        value: 2.5
    TraceSinks:
    - Name: Sink1
      OutputPath: FlexrayDemo_node0.mf4
      Type: Mdf4File
  - Analog-In:
    - Name: AIO
      Replay:
        Direction: Receive
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    CanControllers:
    - Name: CAN1
      Replay:
        Direction: Send
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    Digital-In:
    - Name: DIO
      Replay:
        Direction: Receive
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    EthernetControllers:
    - MacAddr: F6:04:68:71:AA:C1
      Name: ETH0
      UseTraceSinks:
      - Sink1
    FlexRayControllers:
    - ClusterParameters:
        gColdstartAttempts: 8
        gCycleCountMax: 63
        gListenNoise: 2
        gMacroPerCycle: 3636
        gMaxWithoutClockCorrectionFatal: 2
        gMaxWithoutClockCorrectionPassive: 2
        gNumberOfMiniSlots: 291
        gNumberOfStaticSlots: 70
        gPayloadLengthStatic: 16
        gSyncFrameIDCountMax: 15
        gdActionPointOffset: 2
        gdDynamicSlotIdlePhase: 1
        gdMiniSlot: 5
        gdMiniSlotActionPointOffset: 2
        gdStaticSlot: 31
        gdSymbolWindow: 1
        gdSymbolWindowActionPointOffset: 1
        gdTSSTransmitter: 9
        gdWakeupTxActive: 60
        gdWakeupTxIdle: 180
      Name: FlexRay1
      NodeParameters:
        pAllowHaltDueToClock: 1
        pAllowPassiveToActive: 0
        pChannels: AB
        pClusterDriftDamping: 2
        pKeySlotId: 5
        pKeySlotOnlyEnabled: 0
        pKeySlotUsedForStartup: 1
        pKeySlotUsedForSync: 0
        pLatestTx: 249
        pMacroInitialOffsetA: 3
        pMacroInitialOffsetB: 3
        pMicroInitialOffsetA: 6
        pMicroInitialOffsetB: 6
        pMicroPerCycle: 200000
        pOffsetCorrectionOut: 127
        pOffsetCorrectionStart: 3632
        pRateCorrectionOut: 81
        pSamplesPerMicrotick: 2
        pWakeupChannel: A
        pWakeupPattern: 33
        pdAcceptedStartupRange: 212
        pdListenTimeout: 400162
        pdMicrotick: 25ns
    GenericSubscribers:
    - Name: GroundTruth
      Replay:
        Direction: Receive
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    DataSubscribers:
    - Name: DataPubSubGroundTruth
      UseTraceSinks:
      - Sink1
    LinControllers:
    - Name: LIN1
      UseTraceSinks:
      - Sink1
    Logger:
      FlushLevel: Warn
      Sinks:
      - Type: Remote
    Name: Node1
    ParticipantController:
      SyncType: DiscreteTime
    Pattern-In:
    - Name: PATTERN
      Replay:
        Direction: Receive
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
    Pwm-In:
    - Name: PWM
      Replay:
        Direction: Receive
        UseTraceSource: Source1
      UseTraceSinks:
      - Sink1
  - Logger:
      FlushLevel: Debug
      Sinks:
      - Level: Debug
        Type: Stdout
    Name: NetworkSimulator
    NetworkSimulators:
    - Name: FlexRay-Simulator
      Replay:
        Direction: Send
        UseTraceSource: Source1
      SimulatedLinks:
      - FlexRay-Cluster-1
      UseTraceSinks:
      - SinkNs
    ParticipantController:
      SyncType: DiscreteTime
    TraceSinks:
    - Name: SinkNs
      OutputPath: FlexrayDemoGood_replay.mf4
      Type: Mdf4File
    TraceSources:
    - InputPath: FlexrayDemoGood.mf4
      Name: Source1
      Type: Mdf4File
  - Logger:
      FlushLevel: Warn
      Sinks:
      - Type: Remote
    Name: SystemController
  - Description: Passive SystemMonitor that can be optionally attached. Does not participate
      in synchronization.
    Logger:
      FlushLevel: Info
      LogFromRemotes: true
      Sinks:
      - Type: Stdout
    Name: SystemMonitor
  TimeSync:
    TickPeriodNs: 123456
)raw";

class YamlConfigTest : public testing::Test
{
};

using namespace ib::cfg;
TEST_F(YamlConfigTest, yaml_doc_relations)
{
    YamlValidator v;
    //ensure that YAML validation of config blocks works
    EXPECT_TRUE(v.IsRootElement("/ConfigVersion"));
    EXPECT_TRUE(v.IsRootElement("/ConfigName"));
    EXPECT_TRUE(v.IsRootElement("/Description"));
    EXPECT_TRUE(v.IsRootElement("/SimulationSetup"));
    EXPECT_TRUE(v.IsRootElement("/MiddlewareConfig"));
    EXPECT_TRUE(v.IsRootElement("/LaunchConfigurations"));

    EXPECT_FALSE(v.IsRootElement(" /LaunchConfigurations"));
    EXPECT_FALSE(v.IsRootElement("/SimulationSetup/Participants"));

    // simulation setup members
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Participants"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Switches"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Links"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/NetworkSimulators"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/TimeSync"));

    EXPECT_FALSE(v.IsSubelementOf("/SimulationSetup", " /SimulationSetup/TimeSync"));
}
TEST_F(YamlConfigTest, candemo_in_yaml_format)
{


    auto config = Config::FromYamlString(demoYaml);
    EXPECT_TRUE(config.simulationSetup.participants.size() == 5);
    EXPECT_TRUE(config.simulationSetup.timeSync.tickPeriod.count() == 123456);

    const auto node0 = config.simulationSetup.participants.at(0);
    const auto node1 = config.simulationSetup.participants.at(1);
    const auto networkSimulator = config.simulationSetup.participants.at(2);
    const auto systemController = config.simulationSetup.participants.at(3);
    const auto systemMonitor = config.simulationSetup.participants.at(4);

    EXPECT_TRUE(node0.name == "Node0");
    EXPECT_TRUE(node0.canControllers.size() == 1);
    EXPECT_TRUE(node0.canControllers.at(0).name == "CAN1");
    EXPECT_TRUE(node0.linControllers.size() == 1);
    EXPECT_TRUE(node0.linControllers.at(0).name == "LIN1");
    EXPECT_TRUE(node0.flexrayControllers.size() == 1);
    EXPECT_TRUE(node0.flexrayControllers.at(0).name == "FlexRay1");
    EXPECT_TRUE(node0.digitalIoPorts.size() == 1);
    EXPECT_TRUE(node0.digitalIoPorts.at(0).name == "DIO");
    EXPECT_TRUE(node0.analogIoPorts.size() == 1);
    EXPECT_TRUE(node0.analogIoPorts.at(0).name == "AIO");
    EXPECT_TRUE(node0.pwmPorts.size() == 1);
    EXPECT_TRUE(node0.pwmPorts.at(0).name == "PWM");
    EXPECT_TRUE(node0.patternPorts.size() == 1);
    EXPECT_TRUE(node0.patternPorts.at(0).name == "PATTERN");
    EXPECT_TRUE(node0.genericPublishers.size() == 1);
    EXPECT_TRUE(node0.genericPublishers.at(0).name == "GroundTruth");
    EXPECT_TRUE(node0.logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(node1.name == "Node1");
    EXPECT_TRUE(node1.canControllers.size() == 1);
    EXPECT_TRUE(node1.canControllers.at(0).name == "CAN1");
    EXPECT_TRUE(node1.logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(networkSimulator.name == "NetworkSimulator");
    EXPECT_TRUE(networkSimulator.networkSimulators.size() == 1);
    EXPECT_TRUE(networkSimulator.networkSimulators.at(0).name == "FlexRay-Simulator");

    EXPECT_TRUE(systemController.name == "SystemController");
    EXPECT_TRUE(systemController.canControllers.size() == 0);
    EXPECT_TRUE(systemController.logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(systemMonitor.name == "SystemMonitor");
    EXPECT_TRUE(systemMonitor.canControllers.size() == 0);
    EXPECT_TRUE(systemMonitor.logger.sinks.at(0).type == Sink::Type::Stdout);
    EXPECT_TRUE(systemMonitor.logger.logFromRemotes);

    EXPECT_TRUE(config.middlewareConfig.vasio.registry.connectAttempts == 9);
    EXPECT_TRUE(config.middlewareConfig.vasio.registry.hostname == "NotLocalhost");
    EXPECT_TRUE(config.middlewareConfig.vasio.registry.port == 1337);
    EXPECT_TRUE(config.middlewareConfig.vasio.enableDomainSockets == false);
    EXPECT_TRUE(config.middlewareConfig.vasio.tcpQuickAck == true);
    EXPECT_TRUE(config.middlewareConfig.vasio.tcpNoDelay == true);
    EXPECT_TRUE(config.middlewareConfig.vasio.tcpReceiveBufferSize == 3456);
    EXPECT_TRUE(config.middlewareConfig.vasio.tcpSendBufferSize == 3456);

}

TEST_F(YamlConfigTest, validate_unknown_toplevel)
{
    auto yamlString = R"yaml(
ConfigName: CanDemo
ConfigVersion: 0.0.1
Description: Sample configuration for CAN without NetworkSimulator
#typo in a toplevel statement, additional 's'
SimulationsSetup:
)yaml";

    std::stringstream warnings;
    bool yamlValid = Validate(yamlString, warnings);
    std::cout << "Yaml Validate: warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(yamlValid && warnings.str().size() > 0) << "YamlValidator warnings: " << warnings.str();
    EXPECT_TRUE(warnings.str().size() > 0);
}

TEST_F(YamlConfigTest, validate_unnamed_children)
{
    auto yamlString = R"yaml(
SimulationSetup:
  Participants:
    - Name: P1
      CanControllers:
      - Name: CAN1
        UseTraceSinks:
        - Sink1
        #misplaced logger
        Logger:
          Sinks:
          - Type: Remote
      Description:  foo
      #correct place
      Logger:
        Sinks:
        - Type: Remote
)yaml";

    std::stringstream warnings;
    bool yamlValid = Validate(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() <<std::endl;
    EXPECT_TRUE(warnings.str().size() > 0);
}

TEST_F(YamlConfigTest, yaml_native_type_conversions)
{
    using namespace ib::cfg;
    {
        uint16_t a{ 0x815 };
        auto node = to_yaml(a);
        uint16_t b = from_yaml<uint16_t>(node);
        EXPECT_TRUE(a == b);
    }
    {
        std::vector<uint32_t> vec{ 0,1,3,4,5 };
        auto node = to_yaml(vec);
        auto vec2 = from_yaml<std::vector<uint32_t>>(node);
        EXPECT_TRUE(vec == vec2);
    }
    {
        MdfChannel mdf;
        mdf.channelName = "channelName";
        mdf.channelPath = "channelPath";
        mdf.channelSource = "channelSource";
        mdf.groupName = "groupName";
        mdf.groupPath = "groupPath";
        mdf.groupSource = "groupSource";
        auto yaml = to_yaml(mdf);
        auto mdf2 = from_yaml<decltype(mdf)>(yaml);
        EXPECT_TRUE(mdf == mdf2);
    }
    {
        Version version;
        version.major = 31;
        version.minor = 33;
        version.patchLevel = 7;
        YAML::Node node;
        node = version;
        auto repr = node.as<std::string>();
        auto version2 = node.as<Version>();
        EXPECT_TRUE(version == version2);
        // parsing wrong type
        EXPECT_THROW(
            {
                YAML::Node node2;
                node2 = "foobar"; //XXX should throw
                auto version3 = node2.as<Version>();
            }, YAML::BadConversion);

    }
    {
        Logger logger;
        Sink sink;
        logger.logFromRemotes = true;
        sink.type = Sink::Type::File;
        sink.level = ib::mw::logging::Level::Trace;
        sink.logname = "filename";
        logger.sinks.push_back(sink);
        sink.type=Sink::Type::Stdout;
        sink.logname = "";
        logger.sinks.push_back(sink);
        YAML::Node node;
        node = logger;
        //auto repr = node.as<std::string>();
        auto logger2 = node.as<Logger>();
        EXPECT_TRUE(logger == logger2);
    }
    {
        DigitalIoPort port{};
        port.name = "DIO";
        port.initvalue = false;
        YAML::Node node;
        node = port;
        auto port2 = node.as<DigitalIoPort>();
        EXPECT_TRUE(port == port2);
    }
    {
        NetworkSimulator ns;
        ns.name = "netsim";
        ns.replay.useTraceSource = "foo";
        ns.simulatedLinks.push_back("link1");
        YAML::Node node;
        node = ns;
        auto ns2 = node.as<NetworkSimulator>();
        EXPECT_TRUE(ns == ns2);
    }
    {
        Config config{};
        YAML::Node node;
        node = config;
        auto config2 = node.as<Config>();
        EXPECT_TRUE(config == config2);
    }
}

template<typename ControllerT>
static inline void testLegacyName()
{
    //legacy format
    auto node = YAML::Load(R"("Ctrl")");
    auto obj = node.as<ControllerT>();
    EXPECT_TRUE(obj.name == "Ctrl");
    // current format
    obj.name = "";
    node = YAML::Load(R"({"Name": "Ctrl"})");
    obj = node.as<ControllerT>();
    EXPECT_TRUE(obj.name == "Ctrl");
}
TEST_F(YamlConfigTest, yaml_legacy)
{
    testLegacyName<CanController>();
    testLegacyName<LinController>();
    testLegacyName<FlexrayController>();
    // DigitalIoPort
    {
        //legacy format
        auto node = YAML::Load(R"({"PortName": true})");
        auto obj = node.as<DigitalIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == true);
        // current format
        obj.name = "";
        obj.initvalue = false;
        node = YAML::Load(R"({"Name": "PortName", "value": true})");
        obj = node.as<DigitalIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == true);
    }
    // DigitalIoPort
    {
        //legacy format
        auto node = YAML::Load(R"({"PortName": true})");
        auto obj = node.as<DigitalIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == true);
        // current format
        obj.name = "";
        obj.initvalue = false;
        node = YAML::Load(R"({"Name": "PortName", "value": true})");
        obj = node.as<DigitalIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == true);
    }
    // AnalogIoPort
    {
        //legacy format
        auto node = YAML::Load(R"({"PortName": {"value": 123.45, "unit": "V"}})");
        auto obj = node.as<AnalogIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.unit == "V");
        EXPECT_TRUE(obj.initvalue == 123.45);
        // current format
        obj.name = "";
        obj.initvalue = 0;
        node = YAML::Load(R"({"Name": "PortName", "value": 123.45, "unit": "V"})");
        obj = node.as<AnalogIoPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.unit == "V");
        EXPECT_TRUE(obj.initvalue == 123.45);
    }
    // PwmPort
    {
        //legacy format
        auto node = YAML::Load(R"({"PortName": {"freq": {"value": 123.45, "unit": "V"}, "duty": 345.6}})");
        auto obj = node.as<PwmPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.unit == "V");
        EXPECT_TRUE(obj.initvalue.frequency == 123.45);
        EXPECT_TRUE(obj.initvalue.dutyCycle == 345.6);
        // current format
        obj.name = "";
        obj.initvalue = { 0.0 ,0.0 };
        node = YAML::Load(R"({"Name": "PortName", "freq": {"value": 123.45, "unit": "V"}, "duty": 345.6})");
        obj = node.as<PwmPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.unit == "V");
        EXPECT_TRUE(obj.initvalue.frequency == 123.45);
        EXPECT_TRUE(obj.initvalue.dutyCycle == 345.6);
    }
    // Nice Patterns...
    {
        //legacy format
        const auto goal = std::vector<uint8_t>{ 0xde, 0xad, 0xbe, 0xef };
        auto node = YAML::Load(R"({"PortName": "deadbeef"})");
        auto obj = node.as<PatternPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == goal);
        // current format
        obj.name = "";
        obj.initvalue = { };
        node = YAML::Load(R"({"Name": "PortName", "value" :"deadbeef"})");
        obj = node.as<PatternPort>();
        EXPECT_TRUE(obj.name == "PortName");
        EXPECT_TRUE(obj.initvalue == goal);
    }
    // Generic Subscribers
    {
        //legacy format
        auto node = YAML::Load(R"("PortName")");
        auto obj = node.as<GenericPort>();
        EXPECT_TRUE(obj.name == "PortName");
        // current format
        obj.name = "";
        node = YAML::Load(R"({"Name": "PortName"})");
        obj = node.as<GenericPort>();
        EXPECT_TRUE(obj.name == "PortName");
    }
    // Data Subscribers
    {
        auto node = YAML::Load(R"({"Name": "PortName"})");
        auto obj = node.as<DataPort>();
        EXPECT_TRUE(obj.name == "PortName");
    }
}

TEST_F(YamlConfigTest, middleware_vasio_convert)
{
    auto node = YAML::Load(R"(
        "VAsio": {
            "Registry": {
                "Hostname": "not localhost",
                "Port": 1234,
                "Logger": {
                    "Sinks": [
                        {
                            "Type": "Remote"
                        }
                    ]
                },
                "ConnectAttempts": 9
            },
            "TcpNoDelay": true,
            "TcpQuickAck": true,
            "TcpSendBufferSize": 3456,
            "TcpReceiveBufferSize": 3456,
            "EnableDomainSockets": false
        }
    )");
    auto cfg = node.as<MiddlewareConfig>();
    const auto& vasio = cfg.vasio;
    EXPECT_EQ(vasio.registry.connectAttempts, 9);
    EXPECT_EQ(vasio.registry.logger.sinks.at(0).type,
        ib::cfg::Sink::Type::Remote);
    EXPECT_EQ(vasio.registry.hostname, "not localhost");
    EXPECT_EQ(vasio.registry.port, 1234);

    EXPECT_EQ(vasio.enableDomainSockets, false);
    EXPECT_EQ(vasio.tcpNoDelay, true);
    EXPECT_EQ(vasio.tcpQuickAck, true);
    EXPECT_EQ(vasio.tcpSendBufferSize, 3456);
    EXPECT_EQ(vasio.tcpReceiveBufferSize, 3456);

}

} // anonymous namespace
