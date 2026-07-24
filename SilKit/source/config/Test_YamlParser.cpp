// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/YamlParser.hpp"

#include "config/ParticipantConfiguration.hpp"

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
      ChannelSource: MyTestChannel1
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
    OutputPath: FlexrayDemo_txt0.mf4
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
    auto config = Deserialize<ParticipantConfiguration>(completeConfiguration);

    EXPECT_EQ(config.participantName, "Node0");

    EXPECT_EQ(config.canControllers.size(), 2u);
    EXPECT_EQ(config.canControllers.at(0).name, "CAN1");
    ASSERT_FALSE(config.canControllers.at(0).network.has_value());
    EXPECT_EQ(config.canControllers.at(0).useTraceSinks.at(0), "Sink1");
    EXPECT_EQ(config.canControllers.at(0).replay.direction, Replay::Direction::Both);
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.channelName.value(), "MyTestChannel1");
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.channelPath.value(), "path/to/myTestChannel1");
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.channelSource.value(), "MyTestChannel1");
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.groupName.value(), "MyTestGroup");
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.groupPath.value(), "path/to/myTestGroup1");
    EXPECT_EQ(config.canControllers.at(0).replay.mdfChannel.groupSource.value(), "MyTestGroup");

    EXPECT_EQ(config.canControllers.at(1).name, "MyCAN2");
    ASSERT_TRUE(config.canControllers.at(1).network.has_value());
    EXPECT_EQ(config.canControllers.at(1).network.value(), "CAN2");

    EXPECT_EQ(config.linControllers.size(), 1u);
    EXPECT_EQ(config.linControllers.at(0).name, "SimpleEcu1_LIN1");
    ASSERT_TRUE(config.linControllers.at(0).network.has_value());
    EXPECT_EQ(config.linControllers.at(0).network.value(), "LIN1");

    EXPECT_EQ(config.flexrayControllers.size(), 1u);
    EXPECT_EQ(config.flexrayControllers.at(0).name, "FlexRay1");
    ASSERT_FALSE(config.flexrayControllers.at(0).network.has_value());

    EXPECT_EQ(config.dataPublishers.size(), 1u);
    EXPECT_EQ(config.dataPublishers.at(0).name, "Publisher1");
    ASSERT_TRUE(config.dataPublishers.at(0).topic.has_value());
    EXPECT_EQ(config.dataPublishers.at(0).topic.value(), "Temperature");

    EXPECT_EQ(config.rpcServers.size(), 1u);
    EXPECT_EQ(config.rpcClients.size(), 1u);

    EXPECT_EQ(config.logging.sinks.size(), 1u);
    EXPECT_EQ(config.logging.sinks.at(0).type, Sink::Type::File);
    EXPECT_EQ(config.logging.sinks.at(0).level, SilKit::Services::Logging::Level::Critical);
    EXPECT_EQ(config.logging.sinks.at(0).logName, "MyLog1");

    EXPECT_EQ(config.logging.sinks.at(0).format, Sink::Format::Json);

    EXPECT_EQ(config.healthCheck.softResponseTimeout.value(), 500ms);
    EXPECT_EQ(config.healthCheck.hardResponseTimeout.value(), 5000ms);

    EXPECT_EQ(config.tracing.traceSinks.size(), 1u);
    EXPECT_EQ(config.tracing.traceSinks.at(0).name, "Sink1");
    EXPECT_EQ(config.tracing.traceSinks.at(0).outputPath, "FlexrayDemo_txt0.mf4");
    EXPECT_EQ(config.tracing.traceSinks.at(0).type, TraceSink::Type::Mdf4File);
    EXPECT_EQ(config.tracing.traceSources.size(), 1u);
    EXPECT_EQ(config.tracing.traceSources.at(0).name, "Source1");
    EXPECT_EQ(config.tracing.traceSources.at(0).inputPath, "path/to/Source1.mf4");
    EXPECT_EQ(config.tracing.traceSources.at(0).type, TraceSource::Type::Mdf4File);

    EXPECT_EQ(config.extensions.searchPathHints.size(), 2u);
    EXPECT_EQ(config.extensions.searchPathHints.at(0), "path/to/extensions1");
    EXPECT_EQ(config.extensions.searchPathHints.at(1), "path/to/extensions2");

    EXPECT_EQ(config.middleware.connectAttempts, 9);
    EXPECT_EQ(config.middleware.registryUri, "silkit://example.com:1234");
    ASSERT_FALSE(config.middleware.enableDomainSockets);
    ASSERT_TRUE(config.middleware.tcpQuickAck);
    ASSERT_TRUE(config.middleware.tcpNoDelay);
    EXPECT_EQ(config.middleware.tcpReceiveBufferSize, 3456);
    EXPECT_EQ(config.middleware.tcpSendBufferSize, 3456);
    ASSERT_FALSE(config.middleware.registryAsFallbackProxy);
}

TEST_F(Test_YamlParser, yaml_file_sink_defaults_to_json_format)
{
    auto config = Deserialize<ParticipantConfiguration>(R"(
Logging:
  Sinks:
  - Type: File
    LogName: MyLog1
)");
    ASSERT_EQ(config.logging.sinks.size(), 1u);
    EXPECT_EQ(config.logging.sinks.at(0).type, Sink::Type::File);
    EXPECT_EQ(config.logging.sinks.at(0).format, Sink::Format::Json);

    auto configExplicit = Deserialize<ParticipantConfiguration>(R"(
Logging:
  Sinks:
  - Type: File
    Format: Simple
    LogName: MyLog1
)");
    ASSERT_EQ(configExplicit.logging.sinks.size(), 1u);
    EXPECT_EQ(configExplicit.logging.sinks.at(0).type, Sink::Type::File);
    EXPECT_EQ(configExplicit.logging.sinks.at(0).format, Sink::Format::Simple);

    auto configStdout = Deserialize<ParticipantConfiguration>(R"(
Logging:
  Sinks:
  - Type: Stdout
)");
    ASSERT_EQ(configStdout.logging.sinks.size(), 1u);
    EXPECT_EQ(configStdout.logging.sinks.at(0).type, Sink::Type::Stdout);
    EXPECT_EQ(configStdout.logging.sinks.at(0).format, Sink::Format::Simple);
}

TEST_F(Test_YamlParser, yaml_elements_in_random_order)
{
    auto config = Deserialize<ParticipantConfiguration>(R"(
---
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
Description: Order of elements is different
ParticipantName: Node0
schemaVersion: 1347
)");

    EXPECT_EQ(config.participantName, "Node0");
    EXPECT_EQ(config.description, "Order of elements is different");
    EXPECT_EQ(config.schemaVersion, "1347");
    EXPECT_EQ(config.canControllers.size(), 1u);
    EXPECT_EQ(config.canControllers.at(0).name, "CAN1");
    ASSERT_FALSE(config.canControllers.at(0).network.has_value());
}
const auto emptyConfiguration = R"raw(
)raw";

TEST_F(Test_YamlParser, yaml_empty_configuration)
{
    ParticipantConfiguration empty{};
    auto config = Deserialize<ParticipantConfiguration>(emptyConfiguration);
    EXPECT_EQ(empty, config);
}

const auto minimalConfiguration = R"raw(
ParticipantName: Node1
)raw";

TEST_F(Test_YamlParser, yaml_minimal_configuration)
{
    auto config = Deserialize<ParticipantConfiguration>(minimalConfiguration);
    EXPECT_EQ(config.participantName, "Node1");
}

TEST_F(Test_YamlParser, yaml_native_type_conversions)
{
    {
        uint16_t a{0x815};
        auto txt = Serialize<uint16_t>(a);
        uint16_t b = Deserialize<uint16_t>(txt);
        EXPECT_EQ(a, b);
    }
    {
        std::vector<uint32_t> vec{0, 1, 3, 4, 5};
        auto txt = Serialize(vec);
        auto vec2 = Deserialize<std::vector<uint32_t>>(txt);
        EXPECT_EQ(vec, vec2);
    }
    {
        MdfChannel mdf;
        mdf.channelName = "channelName";
        mdf.channelPath = "channelPath";
        mdf.channelSource = "channelSource";
        mdf.groupName = "groupName";
        mdf.groupPath = "groupPath";
        mdf.groupSource = "groupSource";
        auto yaml = Serialize(mdf);
        auto mdf2 = Deserialize<decltype(mdf)>(yaml);
        EXPECT_EQ(mdf, mdf2);
    }
    {
        Logging logger;
        Sink sink;
        logger.logFromRemotes = true;
        sink.type = Sink::Type::File;
        sink.format = Sink::Format::Json;
        sink.level = SilKit::Services::Logging::Level::Trace;
        sink.logName = "filename";
        logger.sinks.push_back(sink);
        sink.type = Sink::Type::Stdout;
        sink.format = Sink::Format::Simple;
        sink.logName = "";
        logger.sinks.push_back(sink);
        auto txt = Serialize(logger);
        auto logger2 = Deserialize<decltype(logger)>(txt);
        EXPECT_EQ(logger, logger2);
    }
    {
        ParticipantConfiguration config{};
        auto txt = Serialize(config);
        auto config2 = Deserialize<decltype(config)>(txt);
        EXPECT_EQ(config, config2);
    }
}

TEST_F(Test_YamlParser, yaml_metrics_update_interval)
{
    // UpdateInterval is read from YAML as an integer count of seconds
    auto config = Deserialize<ParticipantConfiguration>(R"(
Experimental:
  Metrics:
    CollectFromRemote: true
    UpdateInterval: 42
)");
    EXPECT_EQ(config.experimental.metrics.updateInterval, 42s);

    // ... and round-trips through serialization
    auto txt = Serialize(config);
    auto config2 = Deserialize<ParticipantConfiguration>(txt);
    EXPECT_EQ(config2.experimental.metrics.updateInterval, 42s);

    // An absent UpdateInterval keeps the default
    auto configDefault = Deserialize<ParticipantConfiguration>(R"(
Experimental:
  Metrics:
    CollectFromRemote: true
)");
    EXPECT_EQ(configDefault.experimental.metrics.updateInterval, 1s);
}

TEST_F(Test_YamlParser, middleware_convert)
{
    auto config = Deserialize<Middleware>(R"(
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
    EXPECT_EQ(config.registryUri, "silkit://not-localhost:12345");
    EXPECT_EQ(config.connectAttempts, 9);

    EXPECT_EQ(config.enableDomainSockets, false);
    EXPECT_EQ(config.tcpNoDelay, true);
    EXPECT_EQ(config.tcpQuickAck, true);
    EXPECT_EQ(config.tcpSendBufferSize, 3456);
    EXPECT_EQ(config.tcpReceiveBufferSize, 3456);
    EXPECT_EQ(config.registryAsFallbackProxy, false);
}

const auto deprecatedFlexRayControllersConfiguration = R"raw(
FlexRayControllers:
- Name: FlexRay1
- Name: FlexRay2
)raw";

TEST_F(Test_YamlParser, yaml_deprecated_FlexRayControllers_configuration)
{
    const auto participantConfiguration =
        Deserialize<ParticipantConfiguration>(deprecatedFlexRayControllersConfiguration);
    EXPECT_EQ(participantConfiguration.flexrayControllers.size(), 2u);
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
    EXPECT_THROW(
        {
            auto cfg = Deserialize<ParticipantConfiguration>(
                bothFlexrayControllersAndDeprecatedFlexRayControllersConfiguration);
        },
        SilKit::ConfigurationError);
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
    auto participantConfiguration = Deserialize<ParticipantConfiguration>(rpcClientConfiguration);

    ASSERT_EQ(participantConfiguration.rpcClients.size(), 3u);

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
        EXPECT_THROW(
            {
                auto&& brokenConfig = Deserialize<ParticipantConfiguration>(configuration);
                (void)brokenConfig;
            },
            SilKit::ConfigurationError);
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
    auto participantConfiguration = Deserialize<ParticipantConfiguration>(rpcServerConfiguration);

    ASSERT_EQ(participantConfiguration.rpcServers.size(), 3u);

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

TEST_F(Test_YamlParser, yaml_throw_on_missing_keyword) {
    // the config has the pKeySlotId missing, but it is required
    auto configWithoutKeyslot = R"(
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
    #pKeySlotId: 10
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

)";

    EXPECT_THROW(
        {
            auto cfg = Deserialize<ParticipantConfiguration>(
                configWithoutKeyslot);
        },
        SilKit::ConfigurationError);
}

TEST_F(Test_YamlParser, yaml_throw_on_healthcheck_as_sequence)
{
    // HealthCheck is a mapping; the leading "- " mistypes it as a single-element list.
    auto healthCheckAsSequence = R"(
HealthCheck:
  - SoftResponseTimeout: 1
)";

    EXPECT_THROW(
        {
            auto cfg = Deserialize<ParticipantConfiguration>(healthCheckAsSequence);
        },
        SilKit::ConfigurationError);
}

TEST_F(Test_YamlParser, yaml_throw_on_object_as_sequence)
{
    // A mapping mistyped as a sequence must be rejected at every nesting level.
    const std::initializer_list<const char*> mistypedConfigurations = {
        R"(
HealthCheck:
  - SoftResponseTimeout: 1
)",
        R"(
Logging:
  - FlushLevel: Critical
)",
        R"(
Middleware:
  - RegistryUri: silkit://localhost:8500
)",
        R"(
Experimental:
  - Metrics:
      CollectFromRemote: true
)",
        R"(
CanControllers:
- Name: CAN1
  Replay:
    - UseTraceSource: Source1
)",
        R"(
Experimental:
  Metrics:
    - CollectFromRemote: true
)",
    };

    for (const auto configuration : mistypedConfigurations)
    {
        EXPECT_THROW(
            {
                auto&& cfg = Deserialize<ParticipantConfiguration>(configuration);
                (void)cfg;
            },
            SilKit::ConfigurationError);
    }
}

TEST_F(Test_YamlParser, yaml_throw_on_stream_document_as_sequence)
{
    // A single "---" document that is itself a bare sequence is the same mistype.
    auto streamDocAsSequence = R"(---
- SoftResponseTimeout: 1
)";

    EXPECT_THROW(
        {
            auto&& cfg = Deserialize<ParticipantConfiguration>(streamDocAsSequence);
            (void)cfg;
        },
        SilKit::ConfigurationError);
}

TEST_F(Test_YamlParser, yaml_throw_on_multi_document_stream)
{
    // A configuration must be a single YAML document; two "---" documents are rejected.
    auto multiDocument = R"(---
ParticipantName: Node0
---
ParticipantName: Node1
)";

    EXPECT_THROW(
        {
            auto&& cfg = Deserialize<ParticipantConfiguration>(multiDocument);
            (void)cfg;
        },
        SilKit::ConfigurationError);
}

TEST_F(Test_YamlParser, yaml_healthcheck_as_map_is_accepted)
{
    // Regression guard: the correctly-typed object must still parse.
    auto config = Deserialize<ParticipantConfiguration>(R"(
HealthCheck:
  SoftResponseTimeout: 1
  HardResponseTimeout: 2
)");

    ASSERT_TRUE(config.healthCheck.softResponseTimeout.has_value());
    ASSERT_TRUE(config.healthCheck.hardResponseTimeout.has_value());
    EXPECT_EQ(config.healthCheck.softResponseTimeout.value(), 1ms);
    EXPECT_EQ(config.healthCheck.hardResponseTimeout.value(), 2ms);
}

TEST_F(Test_YamlParser, yaml_empty_object_keeps_defaults)
{
    // An empty object (null node) keeps its defaults and must not throw.
    ParticipantConfiguration defaults{};

    auto config = Deserialize<ParticipantConfiguration>(R"(
HealthCheck:
)");

    EXPECT_EQ(config.healthCheck.softResponseTimeout, defaults.healthCheck.softResponseTimeout);
    EXPECT_EQ(config.healthCheck.hardResponseTimeout, defaults.healthCheck.hardResponseTimeout);
}

TEST_F(Test_YamlParser, yaml_sequence_typed_fields_still_parse)
{
    // Genuinely sequence-typed fields (lists of objects) must keep working.
    auto config = Deserialize<ParticipantConfiguration>(R"(
CanControllers:
- Name: CAN1
  Network: CAN2
- Name: CAN2
LinControllers:
- Name: LIN1
)");

    ASSERT_EQ(config.canControllers.size(), 2u);
    EXPECT_EQ(config.canControllers.at(0).name, "CAN1");
    ASSERT_TRUE(config.canControllers.at(0).network.has_value());
    EXPECT_EQ(config.canControllers.at(0).network.value(), "CAN2");
    EXPECT_EQ(config.canControllers.at(1).name, "CAN2");
    ASSERT_EQ(config.linControllers.size(), 1u);
    EXPECT_EQ(config.linControllers.at(0).name, "LIN1");
}

} // anonymous namespace
