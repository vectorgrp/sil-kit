/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "YamlParser.hpp"

#include "ParticipantConfiguration.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <chrono>

namespace {

class Test_YamlParser : public testing::Test
{
};

using namespace SilKit::Config;
using namespace std::chrono_literals;

//! Yaml config which has almost complete list of config elements.
const auto completeConfiguration = R"raw(
---
Description: Example configuration to test YAML Parser
schemaVersion: 0
ParticipantName: Node0
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
LinControllers:
- Name: SimpleEcu1_LIN1
  Network: LIN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - MyTraceSink1
EthernetControllers:
- Name: ETH0
  Replay:
    UseTraceSource: Source1
    Direction: Receive
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - MyTraceSink1
FlexrayControllers:
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
  TxBufferConfigurations:
  - channels: A
    headerCrc: 0
    offset: 0
    PPindicator: false
    repetition: 0
    slotId: 0
    transmissionMode: Continuous
  Replay:
    Direction: Send
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
    UseTraceSource: Source1
  UseTraceSinks:
  - Sink1
DataPublishers:
- Name: Publisher1
  Topic: Temperature
  UseTraceSinks:
  - Sink1
DataSubscribers:
- Name: Subscriber1
  Topic: Temperature
  UseTraceSinks:
  - Sink1
RpcServers:
- Name: Server1
  FunctionName: Function1
  UseTraceSinks:
  - Sink1
RpcClients:
- Name: Client1
  FunctionName: Function1
  UseTraceSinks:
  - Sink1
Logging:
  Sinks:
  - Type: File
    Level: Critical
    LogName: MyLog1
  FlushLevel: Critical
  LogFromRemotes: false
HealthCheck:
  SoftResponseTimeout: 500
  HardResponseTimeout: 5000
Tracing:
  TraceSinks:
  - Name: Sink1
    OutputPath: FlexrayDemo_node0.mf4
    Type: Mdf4File
  TraceSources:
  - Name: Source1
    InputPath: path/to/Source1.mf4
    Type: Mdf4File
Extensions:
  SearchPathHints:
  - path/to/extensions1
  - path/to/extensions2
Middleware:
  RegistryUri: silkit://example.com:1234
  ConnectAttempts: 9
  TcpNoDelay: true
  TcpQuickAck: true
  EnableDomainSockets: false
  TcpSendBufferSize: 3456
  TcpReceiveBufferSize: 3456
  RegistryAsFallbackProxy: false

)raw";

TEST_F(Test_YamlParser, yaml_complete_configuration)
{
    auto node = YAML::Load(completeConfiguration);
    auto config = node.as<ParticipantConfiguration>();

    EXPECT_TRUE(config.participantName == "Node0");

    EXPECT_TRUE(config.canControllers.size() == 2);
    EXPECT_TRUE(config.canControllers.at(0).name == "CAN1");
    EXPECT_TRUE(!config.canControllers.at(0).network.has_value());
    EXPECT_TRUE(config.canControllers.at(1).name == "MyCAN2");
    EXPECT_TRUE(config.canControllers.at(1).network.has_value() && 
        config.canControllers.at(1).network.value() == "CAN2");

    EXPECT_TRUE(config.linControllers.size() == 1);
    EXPECT_TRUE(config.linControllers.at(0).name == "SimpleEcu1_LIN1");
    EXPECT_TRUE(config.linControllers.at(0).network.has_value() && 
        config.linControllers.at(0).network.value() == "LIN1");

    EXPECT_TRUE(config.flexrayControllers.size() == 1);
    EXPECT_TRUE(config.flexrayControllers.at(0).name == "FlexRay1");
    EXPECT_TRUE(!config.flexrayControllers.at(0).network.has_value());

    EXPECT_TRUE(config.dataPublishers.size() == 1);
    EXPECT_TRUE(config.dataPublishers.at(0).name == "Publisher1");
    EXPECT_TRUE(config.dataPublishers.at(0).topic.has_value() && 
        config.dataPublishers.at(0).topic.value() == "Temperature");

    EXPECT_TRUE(config.logging.sinks.size() == 1);
    EXPECT_TRUE(config.logging.sinks.at(0).type == Sink::Type::File);
    EXPECT_TRUE(config.logging.sinks.at(0).level == SilKit::Services::Logging::Level::Critical);
    EXPECT_TRUE(config.logging.sinks.at(0).logName == "MyLog1");

    EXPECT_TRUE(config.healthCheck.softResponseTimeout.value() == 500ms);
    EXPECT_TRUE(config.healthCheck.hardResponseTimeout.value() == 5000ms);

    EXPECT_TRUE(config.tracing.traceSinks.size() == 1);
    EXPECT_TRUE(config.tracing.traceSinks.at(0).name == "Sink1");
    EXPECT_TRUE(config.tracing.traceSinks.at(0).outputPath == "FlexrayDemo_node0.mf4");
    EXPECT_TRUE(config.tracing.traceSinks.at(0).type == TraceSink::Type::Mdf4File);
    EXPECT_TRUE(config.tracing.traceSources.size() == 1);
    EXPECT_TRUE(config.tracing.traceSources.at(0).name == "Source1");
    EXPECT_TRUE(config.tracing.traceSources.at(0).inputPath == "path/to/Source1.mf4");
    EXPECT_TRUE(config.tracing.traceSources.at(0).type == TraceSource::Type::Mdf4File);

    EXPECT_TRUE(config.extensions.searchPathHints.size() == 2);
    EXPECT_TRUE(config.extensions.searchPathHints.at(0) == "path/to/extensions1");
    EXPECT_TRUE(config.extensions.searchPathHints.at(1) == "path/to/extensions2");

    EXPECT_TRUE(config.middleware.connectAttempts == 9);
    EXPECT_TRUE(config.middleware.registryUri == "silkit://example.com:1234");
    EXPECT_TRUE(config.middleware.enableDomainSockets == false);
    EXPECT_TRUE(config.middleware.tcpQuickAck == true);
    EXPECT_TRUE(config.middleware.tcpNoDelay == true);
    EXPECT_TRUE(config.middleware.tcpReceiveBufferSize == 3456);
    EXPECT_TRUE(config.middleware.tcpSendBufferSize == 3456);
    EXPECT_FALSE(config.middleware.registryAsFallbackProxy);
}

const auto emptyConfiguration = R"raw(
)raw";

TEST_F(Test_YamlParser, yaml_empty_configuration)
{
    auto node = YAML::Load(emptyConfiguration);
    EXPECT_THROW({
        try
        {
            node.as<ParticipantConfiguration>();
        }
        catch (const YAML::TypedBadConversion<ParticipantConfiguration>& e)
        {
            EXPECT_STREQ("bad conversion", e.what());
            throw;
        }
    }, YAML::TypedBadConversion<ParticipantConfiguration>);
}

const auto minimalConfiguration = R"raw(
ParticipantName: Node1
)raw";

TEST_F(Test_YamlParser, yaml_minimal_configuration)
{
    auto node = YAML::Load(minimalConfiguration);
    auto config = node.as<ParticipantConfiguration>();
    EXPECT_TRUE(config.participantName == "Node1");
}

TEST_F(Test_YamlParser, yaml_native_type_conversions)
{
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
        Logging logger;
        Sink sink;
        logger.logFromRemotes = true;
        sink.type = Sink::Type::File;
        sink.level = SilKit::Services::Logging::Level::Trace;
        sink.logName = "filename";
        logger.sinks.push_back(sink);
        sink.type = Sink::Type::Stdout;
        sink.logName = "";
        logger.sinks.push_back(sink);
        YAML::Node node;
        node = logger;
        //auto repr = node.as<std::string>();
        auto logger2 = node.as<Logging>();
        EXPECT_TRUE(logger == logger2);
    }
    {
        ParticipantConfiguration config{};
        YAML::Node node;
        node = config;
        auto config2 = node.as<ParticipantConfiguration>();
        EXPECT_TRUE(config == config2);
    }
}

TEST_F(Test_YamlParser, middleware_convert)
{
    auto node = YAML::Load(R"(
        {
            "RegistryUri": "silkit://not-localhost:12345",
            "ConnectAttempts": 9,
            "TcpNoDelay": true,
            "TcpQuickAck": true,
            "TcpSendBufferSize": 3456,
            "TcpReceiveBufferSize": 3456,
            "EnableDomainSockets": false,
            "RegistryAsFallbackProxy": false
        }
    )");
    auto config = node.as<Middleware>();
    EXPECT_EQ(config.registryUri, "silkit://not-localhost:12345");
    EXPECT_EQ(config.connectAttempts, 9);

    EXPECT_EQ(config.enableDomainSockets, false);
    EXPECT_EQ(config.tcpNoDelay, true);
    EXPECT_EQ(config.tcpQuickAck, true);
    EXPECT_EQ(config.tcpSendBufferSize, 3456);
    EXPECT_EQ(config.tcpReceiveBufferSize, 3456);
    EXPECT_EQ(config.registryAsFallbackProxy, false);
}

TEST_F(Test_YamlParser, map_serdes)
{
    std::map<std::string, std::string> mapin{
        {"keya", "vala"}, {"keyb", "valb"}, {"keyc", ""}, {"", "vald"}, 
        {"keye\nwithlinebreak", "vale\nwithlinebreak"}};
    auto mapstr = SilKit::Config::Serialize<std::map<std::string, std::string>>(mapin);
    auto mapout = SilKit::Config::Deserialize<std::map<std::string, std::string>>(mapstr);
    EXPECT_EQ(mapin, mapout);
}

const auto deprecatedFlexRayControllersConfiguration = R"raw(
FlexRayControllers:
- Name: FlexRay1
- Name: FlexRay2
)raw";

TEST_F(Test_YamlParser, yaml_deprecated_FlexRayControllers_configuration)
{
    auto node = YAML::Load(deprecatedFlexRayControllersConfiguration);
    const auto participantConfiguration = node.as<ParticipantConfiguration>();
    EXPECT_EQ(participantConfiguration.flexrayControllers.size(), 2);
    EXPECT_EQ(participantConfiguration.flexrayControllers[0].name, "FlexRay1");
    EXPECT_EQ(participantConfiguration.flexrayControllers[1].name, "FlexRay2");
}

const auto bothFlexrayControllersAndDeprecatedFlexRayControllersConfiguration = R"raw(
FlexRayControllers:
- Name: FlexRay1
- Name: FlexRay2
FlexrayControllers:
- Name: FlexRay3
- Name: FlexRay4
)raw";

// Check that having both the correct "FlexrayControllers" and the deprecated "FlexRayControllers" keys present throws.
TEST_F(Test_YamlParser, yaml_both_FlexrayControllers_and_deprecated_FlexRayControllers_configuration)
{
    auto node = YAML::Load(bothFlexrayControllersAndDeprecatedFlexRayControllersConfiguration);
    EXPECT_THROW({ node.as<ParticipantConfiguration>(); }, ConversionError);
}

const auto rpcClientConfiguration = R"raw(
RpcClients:
- Name: TheRpcClient1
  FunctionName: TheFunction1
- Name: TheRpcClient2
  Channel: TheFunction2
- Name: TheRpcClient3
  RpcChannel: TheFunction3
)raw";

TEST_F(Test_YamlParser, yaml_deprecated_RpcClient_configuration)
{
    auto node = YAML::Load(rpcClientConfiguration);
    const auto participantConfiguration = node.as<ParticipantConfiguration>();

    ASSERT_EQ(participantConfiguration.rpcClients.size(), 3);

    EXPECT_EQ(participantConfiguration.rpcClients[0].name, "TheRpcClient1");
    ASSERT_TRUE(participantConfiguration.rpcClients[0].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcClients[0].functionName.value(), "TheFunction1");

    EXPECT_EQ(participantConfiguration.rpcClients[1].name, "TheRpcClient2");
    ASSERT_TRUE(participantConfiguration.rpcClients[1].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcClients[1].functionName.value(), "TheFunction2");

    EXPECT_EQ(participantConfiguration.rpcClients[2].name, "TheRpcClient3");
    ASSERT_TRUE(participantConfiguration.rpcClients[2].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcClients[2].functionName.value(), "TheFunction3");
}

const auto brokenRpcClientConfigurationA = R"raw(
RpcClients:
- Name: TheRpcClient
  FunctionName: TheFunctionName
  Channel: TheFunctionName
)raw";

const auto brokenRpcClientConfigurationB = R"raw(
RpcClients:
- Name: TheRpcClient
  FunctionName: TheFunctionName
  RpcChannel: TheFunctionName
)raw";

const auto brokenRpcClientConfigurationC = R"raw(
RpcClients:
- Name: TheRpcClient
  Channel: TheFunctionName
  RpcChannel: TheFunctionName
)raw";

const auto brokenRpcClientConfigurationD = R"raw(
RpcClients:
- Name: TheRpcClient
  FunctionName: TheFunctionName
  Channel: TheFunctionName
  RpcChannel: TheFunctionName
)raw";

TEST_F(Test_YamlParser, yaml_broken_RpcClient_configuration)
{
    const std::initializer_list<const char*> brokenConfigurations = {
        brokenRpcClientConfigurationA,
        brokenRpcClientConfigurationB,
        brokenRpcClientConfigurationC,
        brokenRpcClientConfigurationD,
    };
    for (const auto configuration : brokenConfigurations)
    {
        auto node = YAML::Load(configuration);
        EXPECT_THROW({ node.as<ParticipantConfiguration>(); }, ConversionError);
    }
}

const auto rpcServerConfiguration = R"raw(
RpcServers:
- Name: TheRpcServer1
  FunctionName: TheFunction1
- Name: TheRpcServer2
  Channel: TheFunction2
- Name: TheRpcServer3
  RpcChannel: TheFunction3
)raw";

TEST_F(Test_YamlParser, yaml_deprecated_RpcServer_configuration)
{
    auto node = YAML::Load(rpcServerConfiguration);
    const auto participantConfiguration = node.as<ParticipantConfiguration>();

    ASSERT_EQ(participantConfiguration.rpcServers.size(), 3);

    EXPECT_EQ(participantConfiguration.rpcServers[0].name, "TheRpcServer1");
    ASSERT_TRUE(participantConfiguration.rpcServers[0].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcServers[0].functionName.value(), "TheFunction1");

    EXPECT_EQ(participantConfiguration.rpcServers[1].name, "TheRpcServer2");
    ASSERT_TRUE(participantConfiguration.rpcServers[1].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcServers[1].functionName.value(), "TheFunction2");

    EXPECT_EQ(participantConfiguration.rpcServers[2].name, "TheRpcServer3");
    ASSERT_TRUE(participantConfiguration.rpcServers[2].functionName.has_value());
    EXPECT_EQ(participantConfiguration.rpcServers[2].functionName.value(), "TheFunction3");
}

} // anonymous namespace
