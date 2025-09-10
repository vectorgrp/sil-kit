// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "YamlWriter.hpp"

namespace VSilKit {

void YamlWriter::Write(const std::chrono::milliseconds& obj)
{
    Write(std::to_string(obj.count()));
}


void YamlWriter::Write(const std::chrono::nanoseconds& obj)
{
    Write(std::to_string(obj.count()));
}


void YamlWriter::Write(const SilKit::Services::MatchingLabel::Kind& obj)
{
    switch (obj)
    {
    case SilKit::Services::MatchingLabel::Kind::Optional:
        [[fallthrough]];
    case SilKit::Services::MatchingLabel::Kind::Mandatory:
        Write(std::underlying_type_t<SilKit::Services::MatchingLabel::Kind>(obj));
        break;
    default:
        throw SilKit::ConfigurationError{"Write MatchingLabel::Kind: Invalid MatchingLabel::Kind"};
    }
}

void YamlWriter::Write(const SilKit::Services::MatchingLabel& obj)
{
    MakeMap();
    WriteKeyValue("key", obj.key);
    WriteKeyValue("value", obj.value);
    WriteKeyValue("kind", obj.kind);
}


void YamlWriter::Write(const SilKit::Services::Logging::Level& obj)
{
    switch (obj)
    {
    case SilKit::Services::Logging::Level::Critical:
        Write("Critical");
        break;
    case SilKit::Services::Logging::Level::Error:
        Write("Error");
        break;
    case SilKit::Services::Logging::Level::Warn:
        Write("Warn");
        break;
    case SilKit::Services::Logging::Level::Info:
        Write("Info");
        break;
    case SilKit::Services::Logging::Level::Debug:
        Write("Debug");
        break;
    case SilKit::Services::Logging::Level::Trace:
        Write("Trace");
        break;
    case SilKit::Services::Logging::Level::Off:
        Write("Off");
        break;
    }
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayClusterParameters& obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    MakeMap();
    WriteKeyValue("gColdstartAttempts", obj.gColdstartAttempts);
    WriteKeyValue("gCycleCountMax", obj.gCycleCountMax);
    WriteKeyValue("gdActionPointOffset", obj.gdActionPointOffset);
    WriteKeyValue("gdDynamicSlotIdlePhase", obj.gdDynamicSlotIdlePhase);
    WriteKeyValue("gdMiniSlot", obj.gdMiniSlot);
    WriteKeyValue("gdMiniSlotActionPointOffset", obj.gdMiniSlotActionPointOffset);
    WriteKeyValue("gdStaticSlot", obj.gdStaticSlot);
    WriteKeyValue("gdSymbolWindow", obj.gdSymbolWindow);
    WriteKeyValue("gdSymbolWindowActionPointOffset", obj.gdSymbolWindowActionPointOffset);
    WriteKeyValue("gdTSSTransmitter", obj.gdTSSTransmitter);
    WriteKeyValue("gdWakeupTxActive", obj.gdWakeupTxActive);
    WriteKeyValue("gdWakeupTxIdle", obj.gdWakeupTxIdle);
    WriteKeyValue("gListenNoise", obj.gListenNoise);
    WriteKeyValue("gMacroPerCycle", obj.gMacroPerCycle);
    WriteKeyValue("gMaxWithoutClockCorrectionFatal", obj.gMaxWithoutClockCorrectionFatal);
    WriteKeyValue("gMaxWithoutClockCorrectionPassive", obj.gMaxWithoutClockCorrectionPassive);
    WriteKeyValue("gNumberOfMiniSlots", obj.gNumberOfMiniSlots);
    WriteKeyValue("gNumberOfStaticSlots", obj.gNumberOfStaticSlots);
    WriteKeyValue("gPayloadLengthStatic", obj.gPayloadLengthStatic);
    WriteKeyValue("gSyncFrameIDCountMax", obj.gSyncFrameIDCountMax);
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayNodeParameters& obj)
{
    MakeMap();
    WriteKeyValue("pAllowHaltDueToClock", obj.pAllowHaltDueToClock);
    WriteKeyValue("pAllowPassiveToActive", obj.pAllowPassiveToActive);
    WriteKeyValue("pClusterDriftDamping", obj.pClusterDriftDamping);
    WriteKeyValue("pdAcceptedStartupRange", obj.pdAcceptedStartupRange);
    WriteKeyValue("pdListenTimeout", obj.pdListenTimeout);
    WriteKeyValue("pKeySlotId", obj.pKeySlotId);
    WriteKeyValue("pKeySlotOnlyEnabled", obj.pKeySlotOnlyEnabled);
    WriteKeyValue("pKeySlotUsedForStartup", obj.pKeySlotUsedForStartup);
    WriteKeyValue("pKeySlotUsedForSync", obj.pKeySlotUsedForSync);
    WriteKeyValue("pLatestTx", obj.pLatestTx);
    WriteKeyValue("pMacroInitialOffsetA", obj.pMacroInitialOffsetA);
    WriteKeyValue("pMacroInitialOffsetB", obj.pMacroInitialOffsetB);
    WriteKeyValue("pMicroInitialOffsetA", obj.pMicroInitialOffsetA);
    WriteKeyValue("pMicroInitialOffsetB", obj.pMicroInitialOffsetB);
    WriteKeyValue("pMicroPerCycle", obj.pMicroPerCycle);
    WriteKeyValue("pOffsetCorrectionOut", obj.pOffsetCorrectionOut);
    WriteKeyValue("pOffsetCorrectionStart", obj.pOffsetCorrectionStart);
    WriteKeyValue("pRateCorrectionOut", obj.pRateCorrectionOut);
    WriteKeyValue("pWakeupPattern", obj.pWakeupPattern);
    WriteKeyValue("pSamplesPerMicrotick", obj.pSamplesPerMicrotick);
    WriteKeyValue("pWakeupChannel", obj.pWakeupChannel);
    WriteKeyValue("pdMicrotick", obj.pdMicrotick);
    WriteKeyValue("pChannels", obj.pChannels);
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayTxBufferConfig& obj)
{
    MakeMap();
    WriteKeyValue("channels", obj.channels);
    WriteKeyValue("slotId", obj.slotId);
    WriteKeyValue("offset", obj.offset);
    WriteKeyValue("repetition", obj.repetition);
    WriteKeyValue("PPindicator", obj.hasPayloadPreambleIndicator);
    WriteKeyValue("headerCrc", obj.headerCrc);
    WriteKeyValue("transmissionMode", obj.transmissionMode);
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayChannel& obj)
{
    switch (obj)
    {
    case SilKit::Services::Flexray::FlexrayChannel::A:
        Write("A");
        break;
    case SilKit::Services::Flexray::FlexrayChannel::B:
        Write("B");
        break;
    case SilKit::Services::Flexray::FlexrayChannel::AB:
        Write("AB");
        break;
    case SilKit::Services::Flexray::FlexrayChannel::None:
        Write("None");
        break;
    }
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayClockPeriod& obj)
{
    switch (obj)
    {
    case SilKit::Services::Flexray::FlexrayClockPeriod::T12_5NS:
        Write("12.5ns");
        break;
    case SilKit::Services::Flexray::FlexrayClockPeriod::T25NS:
        Write("25ns");
        break;
    case SilKit::Services::Flexray::FlexrayClockPeriod::T50NS:
        Write("50ns");
        break;
    default:
        throw SilKit::ConfigurationError("Unknown Services::Flexray::FlexrayClockPeriod");
    }
}


void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayTransmissionMode& obj)
{
    switch (obj)
    {
    case SilKit::Services::Flexray::FlexrayTransmissionMode::Continuous:
        Write("Continuous");
        break;
    case SilKit::Services::Flexray::FlexrayTransmissionMode::SingleShot:
        Write("SingleShot");
        break;
    default:
        throw SilKit::ConfigurationError("Unknown FlexrayTransmissionMode");
    }
}


void YamlWriter::Write(const SilKit::Config::Sink::Type& obj)
{
    switch (obj)
    {
    case SilKit::Config::Sink::Type::Remote:
        Write("Remote");
        break;
    case SilKit::Config::Sink::Type::Stdout:
        Write("Stdout");
        break;
    case SilKit::Config::Sink::Type::File:
        Write("File");
        break;
    }
}


void YamlWriter::Write(const SilKit::Config::Sink::Format& obj)
{
    switch (obj)
    {
    case SilKit::Config::Sink::Format::Simple:
        Write("Simple");
        break;
    case SilKit::Config::Sink::Format::Json:
        Write("Json");
        break;
    }
}


void YamlWriter::Write(const SilKit::Config::Sink& obj)
{
    static const SilKit::Config::Sink defaultSink{};
    // ParticipantConfiguration.schema.json: Type is required:
    MakeMap();
    WriteKeyValue("Type", obj.type);
    NonDefaultWrite(obj.level, "Level", defaultSink.level);
    NonDefaultWrite(obj.logName, "LogName", defaultSink.logName);
}


void YamlWriter::Write(const SilKit::Config::Logging& obj)
{
    static const SilKit::Config::Logging defaultLogger{};
    MakeMap();
    NonDefaultWrite(obj.logFromRemotes, "LogFromRemotes", defaultLogger.logFromRemotes);
    NonDefaultWrite(obj.flushLevel, "FlushLevel", defaultLogger.flushLevel);
    // ParticipantConfiguration.schema.json: this is a required property:
    WriteKeyValue("Sinks", obj.sinks);
}


// Metrics
void YamlWriter::Write(const SilKit::Config::MetricsSink::Type& obj)
{
    switch (obj)
    {
    case SilKit::Config::MetricsSink::Type::Undefined:
        Write("Undefined");
        break;
    case SilKit::Config::MetricsSink::Type::JsonFile:
        Write("JsonFile");
        break;
    case SilKit::Config::MetricsSink::Type::Remote:
        Write("Remote");
        break;
    default:
        throw SilKit::ConfigurationError{"Unknown MetricsSink Type"};
    }
}


void YamlWriter::Write(const SilKit::Config::MetricsSink& obj)
{
    MakeMap();
    WriteKeyValue("Type", obj.type);
    if (!obj.name.empty())
    {
        WriteKeyValue("Name", obj.name);
    }
}


void YamlWriter::Write(const SilKit::Config::Metrics& obj)
{
    MakeMap();
    OptionalWrite(obj.sinks, "Sinks");
    if (obj.collectFromRemote.has_value())
    {
        WriteKeyValue("CollectFromRemote", obj.collectFromRemote.value());
    }
}


void YamlWriter::Write(const SilKit::Config::MdfChannel& obj)
{
    MakeMap();
    OptionalWrite(obj.channelName, "ChannelName");
    OptionalWrite(obj.channelPath, "ChannelPath");
    OptionalWrite(obj.channelSource, "ChannelSource");
    OptionalWrite(obj.groupName, "GroupName");
    OptionalWrite(obj.groupPath, "GroupPath");
    OptionalWrite(obj.groupSource, "GroupSource");
}


void YamlWriter::Write(const SilKit::Config::Replay& obj)
{
    static const SilKit::Config::Replay defaultObject{};
    MakeMap();
    WriteKeyValue("UseTraceSource", obj.useTraceSource);
    NonDefaultWrite(obj.direction, "Direction", defaultObject.direction);
    NonDefaultWrite(obj.mdfChannel, "MdfChannel", defaultObject.mdfChannel);
}


void YamlWriter::Write(const SilKit::Config::Replay::Direction& obj)
{
    switch (obj)
    {
    case SilKit::Config::Replay::Direction::Send:
        Write("Send");
        break;
    case SilKit::Config::Replay::Direction::Receive:
        Write("Receive");
        break;
    case SilKit::Config::Replay::Direction::Both:
        Write("Both");
        break;
    case SilKit::Config::Replay::Direction::Undefined:
        Write("Undefined");
        break;
    }
}


void YamlWriter::Write(const SilKit::Config::CanController& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.network, "Network");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}


void YamlWriter::Write(const SilKit::Config::LinController& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.network, "Network");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}


void YamlWriter::Write(const SilKit::Config::EthernetController& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.network, "Network");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}


void YamlWriter::Write(const SilKit::Config::FlexrayController& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.network, "Network");
    OptionalWrite(obj.clusterParameters, "ClusterParameters");
    OptionalWrite(obj.nodeParameters, "NodeParameters");
    OptionalWrite(obj.txBufferConfigurations, "TxBufferConfigurations");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}

void YamlWriter::Write(const SilKit::Config::Label::Kind& obj)
{
    switch (obj)
    {
    case SilKit::Config::Label::Kind::Mandatory:
        Write("Mandatory");
        break;
    case SilKit::Config::Label::Kind::Optional:
        Write("Optional");
        break;
    default:
        throw SilKit::ConfigurationError{"Unknown Label::Kind"};
    }
}


void YamlWriter::Write(const SilKit::Config::Label& obj)
{
    MakeMap();
    WriteKeyValue("Key", obj.key);
    WriteKeyValue("Value", obj.value);
    WriteKeyValue("Kind", obj.kind);
}

void YamlWriter::Write(const SilKit::Config::DataPublisher& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.topic, "Topic");
    OptionalWrite(obj.labels, "Labels");
    OptionalWrite(obj.history, "History");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}


void YamlWriter::Write(const SilKit::Config::DataSubscriber& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.topic, "Topic");
    OptionalWrite(obj.labels, "Labels");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}

void YamlWriter::Write(const SilKit::Config::RpcServer& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.functionName, "FunctionName");
    OptionalWrite(obj.labels, "Labels");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}


void YamlWriter::Write(const SilKit::Config::RpcClient& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    OptionalWrite(obj.functionName, "Channel");
    OptionalWrite(obj.labels, "Labels");
    OptionalWrite(obj.useTraceSinks, "UseTraceSinks");
    OptionalWrite(obj.replay, "Replay");
}

void YamlWriter::Write(const SilKit::Config::Tracing& obj)
{
    MakeMap();
    OptionalWrite(obj.traceSinks, "TraceSinks");
    OptionalWrite(obj.traceSources, "TraceSources");
}

void YamlWriter::Write(const SilKit::Config::TraceSink& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    WriteKeyValue("Type", obj.type);
    WriteKeyValue("OutputPath", obj.outputPath);
}


void YamlWriter::Write(const SilKit::Config::TraceSink::Type& obj)
{
    switch (obj)
    {
    case SilKit::Config::TraceSink::Type::Undefined:
        Write("Undefined");
        break;
    case SilKit::Config::TraceSink::Type::Mdf4File:
        Write("Mdf4File");
        break;
    case SilKit::Config::TraceSink::Type::PcapFile:
        Write("PcapFile");
        break;
    case SilKit::Config::TraceSink::Type::PcapPipe:
        Write("PcapPipe");
        break;
    default:
        throw SilKit::ConfigurationError{"Unknown TraceSink Type"};
    }
}


void YamlWriter::Write(const SilKit::Config::TraceSource& obj)
{
    MakeMap();
    WriteKeyValue("Name", obj.name);
    WriteKeyValue("Type", obj.type);
    WriteKeyValue("InputPath", obj.inputPath);
}


void YamlWriter::Write(const SilKit::Config::TraceSource::Type& obj)
{
    switch (obj)
    {
    case SilKit::Config::TraceSource::Type::Undefined:
        Write("Undefined");
        break;
    case SilKit::Config::TraceSource::Type::Mdf4File:
        Write("Mdf4File");
        break;
    case SilKit::Config::TraceSource::Type::PcapFile:
        Write("PcapFile");
        break;
    default:
        throw SilKit::ConfigurationError{"Unknown TraceSource Type"};
    }
}


void YamlWriter::Write(const SilKit::Config::Extensions& obj)
{
    static const SilKit::Config::Extensions defaultObj{};
    MakeMap();
    NonDefaultWrite(obj.searchPathHints, "SearchPathHints", defaultObj.searchPathHints);
}


void YamlWriter::Write(const SilKit::Config::Middleware& obj)
{
    static const SilKit::Config::Middleware defaultObj;
    MakeMap();
    NonDefaultWrite(obj.registryUri, "RegistryUri", defaultObj.registryUri);
    NonDefaultWrite(obj.connectAttempts, "ConnectAttempts", defaultObj.connectAttempts);
    NonDefaultWrite(obj.tcpNoDelay, "TcpNoDelay", defaultObj.tcpNoDelay);
    NonDefaultWrite(obj.tcpQuickAck, "TcpQuickAck", defaultObj.tcpQuickAck);
    NonDefaultWrite(obj.tcpReceiveBufferSize, "TcpReceiveBufferSize", defaultObj.tcpReceiveBufferSize);
    NonDefaultWrite(obj.tcpSendBufferSize, "TcpSendBufferSize", defaultObj.tcpSendBufferSize);
    NonDefaultWrite(obj.enableDomainSockets, "EnableDomainSockets", defaultObj.enableDomainSockets);
    NonDefaultWrite(obj.acceptorUris, "AcceptorUris", defaultObj.acceptorUris);
    NonDefaultWrite(obj.registryAsFallbackProxy, "RegistryAsFallbackProxy", defaultObj.registryAsFallbackProxy);
    NonDefaultWrite(obj.experimentalRemoteParticipantConnection, "ExperimentalRemoteParticipantConnection",
                    defaultObj.experimentalRemoteParticipantConnection);
    NonDefaultWrite(obj.connectTimeoutSeconds, "ConnectTimeoutSeconds", defaultObj.connectTimeoutSeconds);
}


void YamlWriter::Write(const SilKit::Config::Includes& obj)
{
    MakeMap();
    OptionalWrite(obj.files, "Files");
    OptionalWrite(obj.searchPathHints, "SearchPathHints");
}

void YamlWriter::Write(const SilKit::Config::Aggregation& obj)
{
    switch (obj)
    {
    case SilKit::Config::Aggregation::Off:
        Write("Off");
        break;
    case SilKit::Config::Aggregation::On:
        Write("On");
        break;
    case SilKit::Config::Aggregation::Auto:
        Write("Auto");
        break;
    default:
        throw SilKit::ConfigurationError{"Unknown Aggregation Type"};
    }
}

void YamlWriter::Write(const SilKit::Config::TimeSynchronization& obj)
{
    static const SilKit::Config::TimeSynchronization defaultObj;
    MakeMap();
    NonDefaultWrite(obj.animationFactor, "AnimationFactor", defaultObj.animationFactor);
    NonDefaultWrite(obj.enableMessageAggregation, "EnableMessageAggregation", defaultObj.enableMessageAggregation);
}


void YamlWriter::Write(const SilKit::Config::Experimental& obj)
{
    static const SilKit::Config::Experimental defaultObj{};

    MakeMap();
    NonDefaultWrite(obj.timeSynchronization, "TimeSynchronization", defaultObj.timeSynchronization);
    NonDefaultWrite(obj.metrics, "Metrics", defaultObj.metrics);
}


void YamlWriter::Write(const SilKit::Config::ParticipantConfiguration& obj)
{
    static const SilKit::Config::ParticipantConfiguration defaultObj{};
    MakeMap();
    WriteKeyValue("SchemaVersion", SilKit::Config::V1::GetSchemaVersion());
    OptionalWrite(obj.description, "Description");
    OptionalWrite(obj.participantName, "ParticipantName");
    OptionalWrite(obj.canControllers, "CanControllers");
    OptionalWrite(obj.linControllers, "LinControllers");
    OptionalWrite(obj.ethernetControllers, "EthernetControllers");
    OptionalWrite(obj.flexrayControllers, "FlexrayControllers");
    OptionalWrite(obj.dataPublishers, "DataPublishers");
    OptionalWrite(obj.dataSubscribers, "DataSubscribers");
    OptionalWrite(obj.rpcServers, "RpcServers");
    OptionalWrite(obj.rpcClients, "RpcClients");

    NonDefaultWrite(obj.logging, "Logging", defaultObj.logging);
    NonDefaultWrite(obj.healthCheck, "HealthCheck", defaultObj.healthCheck);
    NonDefaultWrite(obj.tracing, "Tracing", defaultObj.tracing);
    NonDefaultWrite(obj.extensions, "Extensions", defaultObj.extensions);
    NonDefaultWrite(obj.middleware, "Middleware", defaultObj.middleware);
    NonDefaultWrite(obj.includes, "Includes", defaultObj.includes);
    NonDefaultWrite(obj.experimental, "Experimental", defaultObj.experimental);
}

void YamlWriter::Write(const SilKit::Config::HealthCheck& obj)
{
    MakeMap();
    OptionalWrite(obj.softResponseTimeout, "SoftResponseTimeout");
    OptionalWrite(obj.hardResponseTimeout, "HardResponseTimeout");
}


void YamlWriter::Write(const SilKitRegistry::Config::V1::Experimental& obj)
{
    static const SilKitRegistry::Config::V1::Experimental defaultObject{};
    NonDefaultWrite(obj.metrics, "Metrics", defaultObject.metrics);
}


void YamlWriter::Write(const SilKitRegistry::Config::V1::RegistryConfiguration& obj)
{
    static const SilKitRegistry::Config::V1::RegistryConfiguration defaultObject{};
    MakeMap();
    WriteKeyValue("SchemaVersion", SilKitRegistry::Config::V1::GetSchemaVersion());
    NonDefaultWrite(obj.description, "Description", defaultObject.description);
    OptionalWrite(obj.listenUri, "ListenUri");
    OptionalWrite(obj.enableDomainSockets, "EnableDomainSockets");
    OptionalWrite(obj.dashboardUri, "DashboardUri");
    NonDefaultWrite(obj.logging, "Logging", defaultObject.logging);
    NonDefaultWrite(obj.experimental, "Experimental", defaultObject.experimental);
}

void YamlWriter::Write(const SilKit::Services::Flexray::FlexrayControllerConfig& obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    MakeMap();
    WriteKeyValue("clusterParams", obj.clusterParams);
    WriteKeyValue("nodeParams", obj.nodeParams);
    OptionalWrite(obj.bufferConfigs, "bufferConfigs");
}

} // namespace VSilKit
