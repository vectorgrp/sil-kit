// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT
#include "YamlReader.hpp"

namespace VSilKit {

bool IsValidChild(const ryml::ConstNodeRef& node, const std::string& name)
{
    return node.is_map() && !node.find_child(ryml::to_csubstr(name)).invalid();
}

bool YamlReader::IsValid() const
{
    return !_context.node.invalid();
}

bool YamlReader::IsMap() const
{
    return _context.node.is_map();
}

bool YamlReader::IsScalar() const
{
    return _context.node.is_val() || _context.node.is_keyval();
}
bool YamlReader::IsSequence() const
{
    return _context.node.is_seq();
}
bool YamlReader::IsExistingString(const char* str) const
{
    if (!_context.node.is_val())
    {
        return false;
    }
    return _context.node.val() == ryml::to_csubstr(str);
}

bool YamlReader::IsEmpty() const
{
    return _context.node.empty();
}

bool YamlReader::IsString(const char* string) const
{
    return IsScalar() && (_context.node.val() == ryml::to_csubstr(string));
}

auto YamlReader::MakeConfigurationError(const char* message) const -> SilKit::ConfigurationError
{
    const auto location = _context.parser.location(_context.node);

    std::ostringstream s;

    s << "error parsing configuration";
    if (location.name.empty())
    {
        s << " file " << location.name << ": ";
    }
    else
    {
        s << " string: ";
    }

    s << "line " << location.line << " column " << location.col << ": " << message;

    return SilKit::ConfigurationError{s.str()};
}

auto YamlReader::MakeYamlReader(ryml::ConstNodeRef node) const -> YamlReader
{
    auto ctx = _context;
    ctx.node = node;
    return {ctx};
}

auto YamlReader::GetChildSafe(const std::string& name) const -> YamlReader
{
    if (IsValidChild(_context.node, name))
    {
        return MakeYamlReader(_context.node.find_child(ryml::to_csubstr(name)));
    }
    else if (IsSequence())
    {
        for (const auto& child : _context.node.cchildren())
        {
            if (child.is_container() && IsValidChild(child, name))
            {
                return MakeYamlReader(child.find_child(ryml::to_csubstr(name)));
            }
        }
    }
    return MakeYamlReader({});
}
void YamlReader::OptionalRead(bool& val, const std::string& name)
{
    auto&& child = GetChildSafe(name);
    if (child.IsValid())
    {
        child.Read(val);
    }
}

void YamlReader::Read(SilKit::Services::MatchingLabel& value)
{
    OptionalRead(value.key, "key");
    OptionalRead(value.kind, "kind");
    OptionalRead(value.value, "value");
}

void YamlReader::Read(SilKit::Services::MatchingLabel::Kind& value)
{
    if (IsString("Mandatory"))
        value = SilKit::Services::MatchingLabel::Kind::Mandatory;
    else if (IsString("Optional"))
        value = SilKit::Services::MatchingLabel::Kind::Optional;
    else
        throw MakeConfigurationError("Kind should be a string of Mandatory|Optional.");
}


void YamlReader::Read(SilKit::Services::Logging::Level& obj)
{
    if (IsString("Critical"))
        obj = SilKit::Services::Logging::Level::Critical;
    else if (IsString("Error"))
        obj = SilKit::Services::Logging::Level::Error;
    else if (IsString("Warn"))
        obj = SilKit::Services::Logging::Level::Warn;
    else if (IsString("Info"))
        obj = SilKit::Services::Logging::Level::Info;
    else if (IsString("Debug"))
        obj = SilKit::Services::Logging::Level::Debug;
    else if (IsString("Trace"))
        obj = SilKit::Services::Logging::Level::Trace;
    else if (IsString("Off"))
        obj = SilKit::Services::Logging::Level::Off;
    else
    {
        throw MakeConfigurationError("Unknown SilKit::Services::Logging::Level.");
    }
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayClusterParameters& obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    ReadKeyValue(obj.gColdstartAttempts, "gColdstartAttempts");
    ReadKeyValue(obj.gCycleCountMax, "gCycleCountMax");
    ReadKeyValue(obj.gdActionPointOffset, "gdActionPointOffset");
    ReadKeyValue(obj.gdDynamicSlotIdlePhase, "gdDynamicSlotIdlePhase");
    ReadKeyValue(obj.gdMiniSlot, "gdMiniSlot");
    ReadKeyValue(obj.gdMiniSlotActionPointOffset, "gdMiniSlotActionPointOffset");
    ReadKeyValue(obj.gdStaticSlot, "gdStaticSlot");
    ReadKeyValue(obj.gdSymbolWindow, "gdSymbolWindow");
    ReadKeyValue(obj.gdSymbolWindowActionPointOffset, "gdSymbolWindowActionPointOffset");
    ReadKeyValue(obj.gdTSSTransmitter, "gdTSSTransmitter");
    ReadKeyValue(obj.gdWakeupTxActive, "gdWakeupTxActive");
    ReadKeyValue(obj.gdWakeupTxIdle, "gdWakeupTxIdle");
    ReadKeyValue(obj.gListenNoise, "gListenNoise");
    ReadKeyValue(obj.gMacroPerCycle, "gMacroPerCycle");
    ReadKeyValue(obj.gMaxWithoutClockCorrectionFatal, "gMaxWithoutClockCorrectionFatal");
    ReadKeyValue(obj.gMaxWithoutClockCorrectionPassive, "gMaxWithoutClockCorrectionPassive");
    ReadKeyValue(obj.gNumberOfMiniSlots, "gNumberOfMiniSlots");
    ReadKeyValue(obj.gNumberOfStaticSlots, "gNumberOfStaticSlots");
    ReadKeyValue(obj.gPayloadLengthStatic, "gPayloadLengthStatic");
    ReadKeyValue(obj.gSyncFrameIDCountMax, "gSyncFrameIDCountMax");
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayNodeParameters& obj)
{
    ReadKeyValue(obj.pAllowHaltDueToClock, "pAllowHaltDueToClock");
    ReadKeyValue(obj.pAllowPassiveToActive, "pAllowPassiveToActive");
    ReadKeyValue(obj.pClusterDriftDamping, "pClusterDriftDamping");
    ReadKeyValue(obj.pdAcceptedStartupRange, "pdAcceptedStartupRange");
    ReadKeyValue(obj.pdListenTimeout, "pdListenTimeout");
    ReadKeyValue(obj.pKeySlotId, "pKeySlotId");
    ReadKeyValue(obj.pKeySlotOnlyEnabled, "pKeySlotOnlyEnabled");
    ReadKeyValue(obj.pKeySlotUsedForStartup, "pKeySlotUsedForStartup");
    ReadKeyValue(obj.pKeySlotUsedForSync, "pKeySlotUsedForSync");
    ReadKeyValue(obj.pLatestTx, "pLatestTx");
    ReadKeyValue(obj.pMacroInitialOffsetA, "pMacroInitialOffsetA");
    ReadKeyValue(obj.pMacroInitialOffsetB, "pMacroInitialOffsetB");
    ReadKeyValue(obj.pMicroInitialOffsetA, "pMicroInitialOffsetA");
    ReadKeyValue(obj.pMicroInitialOffsetB, "pMicroInitialOffsetB");
    ReadKeyValue(obj.pMicroPerCycle, "pMicroPerCycle");
    ReadKeyValue(obj.pOffsetCorrectionOut, "pOffsetCorrectionOut");
    ReadKeyValue(obj.pOffsetCorrectionStart, "pOffsetCorrectionStart");
    ReadKeyValue(obj.pRateCorrectionOut, "pRateCorrectionOut");
    ReadKeyValue(obj.pWakeupPattern, "pWakeupPattern");
    ReadKeyValue(obj.pSamplesPerMicrotick, "pSamplesPerMicrotick");
    ReadKeyValue(obj.pWakeupChannel, "pWakeupChannel");
    ReadKeyValue(obj.pdMicrotick, "pdMicrotick");
    ReadKeyValue(obj.pChannels, "pChannels");
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayTxBufferConfig& obj)
{
    ReadKeyValue(obj.channels, "channels");
    ReadKeyValue(obj.slotId, "slotId");
    ReadKeyValue(obj.offset, "offset");
    ReadKeyValue(obj.repetition, "repetition");
    ReadKeyValue(obj.hasPayloadPreambleIndicator, "PPindicator");
    ReadKeyValue(obj.headerCrc, "headerCrc");
    ReadKeyValue(obj.transmissionMode, "transmissionMode");
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayChannel& obj)
{
    if (IsString("A"))
        obj = SilKit::Services::Flexray::FlexrayChannel::A;
    else if (IsString("B"))
        obj = SilKit::Services::Flexray::FlexrayChannel::B;
    else if (IsString("AB"))
        obj = SilKit::Services::Flexray::FlexrayChannel::AB;
    else if (IsString("None") || IsString(""))
        obj = SilKit::Services::Flexray::FlexrayChannel::None;
    else
    {
        throw MakeConfigurationError("Unknown Services::Flexray::FlexrayChannel");
    }
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayClockPeriod& obj)
{
    if (IsString("12.5ns"))
        obj = SilKit::Services::Flexray::FlexrayClockPeriod::T12_5NS;
    else if (IsString("25ns"))
        obj = SilKit::Services::Flexray::FlexrayClockPeriod::T25NS;
    else if (IsString("50ns"))
        obj = SilKit::Services::Flexray::FlexrayClockPeriod::T50NS;
    else
    {
        throw MakeConfigurationError("Unknown Services::Flexray::FlexrayClockPeriod");
    }
}

void YamlReader::Read(SilKit::Services::Flexray::FlexrayTransmissionMode& obj)
{
    if (IsString("Continuous"))
        obj = SilKit::Services::Flexray::FlexrayTransmissionMode::Continuous;
    else if (IsString("SingleShot"))
        obj = SilKit::Services::Flexray::FlexrayTransmissionMode::SingleShot;
    else
    {
        throw MakeConfigurationError("Unknown Services::Flexray::FlexrayTransmissionMode.");
    }
}

void YamlReader::Read(SilKit::Config::Sink::Type& obj)
{
    if (IsString("Remote") || IsString(""))
    {
        obj = SilKit::Config::Sink::Type::Remote;
    }
    else if (IsString("Stdout"))
    {
        obj = SilKit::Config::Sink::Type::Stdout;
    }
    else if (IsString("File"))
    {
        obj = SilKit::Config::Sink::Type::File;
    }
    else
    {
        throw MakeConfigurationError("Unknown Sink::Type");
    }
}

void YamlReader::Read(SilKit::Config::Sink::Format& obj)
{
    if (IsString("Simple") || IsString(""))
    {
        obj = SilKit::Config::Sink::Format::Simple;
    }
    else if (IsString("Json"))
    {
        obj = SilKit::Config::Sink::Format::Json;
    }
    else
    {
        throw MakeConfigurationError("Unknown Sink::Format:");
    }
}

void YamlReader::Read(SilKit::Config::Sink& obj)
{
    OptionalRead(obj.type, "Type");
    OptionalRead(obj.level, "Level");
    OptionalRead(obj.format, "Format");

    if (obj.type == SilKit::Config::Sink::Type::File)
    {
        if (!IsValidChild(_context.node, "LogName"))
        {
            throw MakeConfigurationError("Sink of type Sink::Type::File requires a LogName");
        }
        ReadKeyValue(obj.logName, "LogName");
    }
}

void YamlReader::Read(SilKit::Config::Logging& obj)
{
    OptionalRead(obj.logFromRemotes, "LogFromRemotes");
    OptionalRead(obj.flushLevel, "FlushLevel");
    OptionalRead(obj.sinks, "Sinks");
}

void YamlReader::Read(SilKit::Config::MetricsSink::Type& obj)
{
    if (IsString("Undefined") || IsString(""))
    {
        obj = SilKit::Config::MetricsSink::Type::Undefined;
    }
    else if (IsString("JsonFile"))
    {
        obj = SilKit::Config::MetricsSink::Type::JsonFile;
    }
    else if (IsString("Remote"))
    {
        obj = SilKit::Config::MetricsSink::Type::Remote;
    }
    else
    {
        throw MakeConfigurationError("Unknown MetricsSink::Type");
    }
}

void YamlReader::Read(SilKit::Config::MetricsSink& obj)
{
    ReadKeyValue(obj.type, "Type");
    OptionalRead(obj.name, "Name");
}

void YamlReader::Read(SilKit::Config::Metrics& obj)
{
    OptionalRead(obj.sinks, "Sinks");
    OptionalRead(obj.collectFromRemote, "CollectFromRemote");

    if (obj.collectFromRemote)
    {
        for (auto&& sink : obj.sinks)
        {
            if (sink.type == SilKit::Config::MetricsSink::Type::Remote)
            {
                throw SilKit::ConfigurationError{
                    "Metrics collectFromRemote is enabled while having a Remote MetricsSink active"};
            }
        }
    }
}

void YamlReader::Read(SilKit::Config::MdfChannel& obj)
{
    if (!IsMap())
    {
        throw MakeConfigurationError("MdfChannel should be a Map");
    }
    OptionalRead(obj.channelName, "ChannelName");
    OptionalRead(obj.channelPath, "ChannelPath");
    OptionalRead(obj.channelSource, "ChannelSource");
    OptionalRead(obj.groupName, "GroupName");
    OptionalRead(obj.groupPath, "GroupPath");
    OptionalRead(obj.groupSource, "GroupSource");
}

void YamlReader::Read(SilKit::Config::Replay& obj)
{
    ReadKeyValue(obj.useTraceSource, "UseTraceSource");
    OptionalRead(obj.direction, "Direction");
    OptionalRead(obj.mdfChannel, "MdfChannel");
}

void YamlReader::Read(SilKit::Config::Replay::Direction& obj)
{
    if (IsString("Undefined") || IsString(""))
        obj = SilKit::Config::Replay::Direction::Undefined;
    else if (IsString("Send"))
        obj = SilKit::Config::Replay::Direction::Send;
    else if (IsString("Receive"))
        obj = SilKit::Config::Replay::Direction::Receive;
    else if (IsString("Both"))
        obj = SilKit::Config::Replay::Direction::Both;
    else
    {
        throw MakeConfigurationError("Unknown Replay::Direction");
    }
}
void YamlReader::Read(SilKit::Config::CanController& obj)
{
    ReadController(obj);
}

void YamlReader::Read(SilKit::Config::LinController& obj)
{
    ReadController(obj);
}

void YamlReader::Read(SilKit::Config::EthernetController& obj)
{
    ReadController(obj);
}

void YamlReader::Read(SilKit::Config::FlexrayController& obj)
{
    ReadController(obj);
    OptionalRead(obj.clusterParameters, "ClusterParameters");
    OptionalRead(obj.nodeParameters, "NodeParameters");
    OptionalRead(obj.txBufferConfigurations, "TxBufferConfigurations");
}

void YamlReader::Read(SilKit::Config::Label::Kind& obj)
{
    if (IsString("Mandatory"))
        obj = SilKit::Config::Label::Kind::Mandatory;
    else if (IsString("Optional"))
        obj = SilKit::Config::Label::Kind::Optional;
    else
    {
        throw MakeConfigurationError("Unknown Label::Kind");
    }
}

void YamlReader::Read(SilKit::Config::Label& obj)
{
    OptionalRead(obj.key, "Key");
    OptionalRead(obj.value, "Value");
    OptionalRead(obj.kind, "Kind");
}

void YamlReader::Read(SilKit::Config::DataPublisher& obj)
{
    ReadKeyValue(obj.name, "Name");
    OptionalRead(obj.topic, "Topic");
    OptionalRead(obj.labels, "Labels");
    OptionalRead(obj.useTraceSinks, "UseTraceSinks");
    OptionalRead(obj.replay, "Replay");
}

void YamlReader::Read(SilKit::Config::DataSubscriber& obj)
{
    ReadKeyValue(obj.name, "Name");
    OptionalRead(obj.topic, "Topic");
    OptionalRead(obj.labels, "Labels");
    OptionalRead(obj.useTraceSinks, "UseTraceSinks");
    OptionalRead(obj.replay, "Replay");
}
void YamlReader::Read(SilKit::Config::RpcServer& obj)
{
    ReadKeyValue(obj.name, "Name");

    OptionalRead_deprecated_alternative(obj.functionName, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj.labels, "Labels");
    OptionalRead(obj.useTraceSinks, "UseTraceSinks");
    OptionalRead(obj.replay, "Replay");
}

void YamlReader::Read(SilKit::Config::RpcClient& obj)
{
    ReadKeyValue(obj.name, "Name");
    OptionalRead_deprecated_alternative(obj.functionName, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj.labels, "Labels");
    OptionalRead(obj.useTraceSinks, "UseTraceSinks");
    OptionalRead(obj.replay, "Replay");
}

void YamlReader::Read(SilKit::Config::Tracing& obj)
{
    OptionalRead(obj.traceSinks, "TraceSinks");
    OptionalRead(obj.traceSources, "TraceSources");
}

void YamlReader::Read(SilKit::Config::TraceSink& obj)
{
    ReadKeyValue(obj.name, "Name");
    ReadKeyValue(obj.type, "Type");
    ReadKeyValue(obj.outputPath, "OutputPath");
}


void YamlReader::Read(SilKit::Config::TraceSink::Type& obj)
{
    if (IsString("Undefined") || IsString(""))
        obj = SilKit::Config::TraceSink::Type::Undefined;
    else if (IsString("Mdf4File"))
        obj = SilKit::Config::TraceSink::Type::Mdf4File;
    else if (IsString("PcapFile"))
        obj = SilKit::Config::TraceSink::Type::PcapFile;
    else if (IsString("PcapPipe"))
        obj = SilKit::Config::TraceSink::Type::PcapPipe;
    else
    {
        throw MakeConfigurationError("Unknown TraceSink::Type");
    }
}
void YamlReader::Read(SilKit::Config::TraceSource& obj)
{
    ReadKeyValue(obj.name, "Name");
    ReadKeyValue(obj.type, "Type");
    ReadKeyValue(obj.inputPath, "InputPath");
}

void YamlReader::Read(SilKit::Config::TraceSource::Type& obj)
{
    if (IsString("Undefined") || IsString(""))
        obj = SilKit::Config::TraceSource::Type::Undefined;
    else if (IsString("Mdf4File"))
        obj = SilKit::Config::TraceSource::Type::Mdf4File;
    else if (IsString("PcapFile"))
        obj = SilKit::Config::TraceSource::Type::PcapFile;
    else
    {
        throw MakeConfigurationError("Unknown TraceSource::Type:");
    }
}

void YamlReader::Read(SilKit::Config::Extensions& obj)
{
    OptionalRead(obj.searchPathHints, "SearchPathHints");
}

void YamlReader::Read(SilKit::Config::Middleware& obj)
{
    OptionalRead(obj.registryUri, "RegistryUri");
    OptionalRead(obj.connectAttempts, "ConnectAttempts");
    OptionalRead(obj.tcpNoDelay, "TcpNoDelay");
    OptionalRead(obj.tcpQuickAck, "TcpQuickAck");
    OptionalRead(obj.tcpReceiveBufferSize, "TcpReceiveBufferSize");
    OptionalRead(obj.tcpSendBufferSize, "TcpSendBufferSize");
    OptionalRead(obj.enableDomainSockets, "EnableDomainSockets");
    OptionalRead(obj.acceptorUris, "AcceptorUris");
    OptionalRead(obj.registryAsFallbackProxy, "RegistryAsFallbackProxy");
    OptionalRead(obj.experimentalRemoteParticipantConnection, "ExperimentalRemoteParticipantConnection");
    OptionalRead(obj.connectTimeoutSeconds, "ConnectTimeoutSeconds");
}

void YamlReader::Read(SilKit::Config::Includes& obj)
{
    OptionalRead(obj.files, "Files");
    OptionalRead(obj.searchPathHints, "SearchPathHints");
}

void YamlReader::Read(SilKit::Config::Aggregation& obj)
{
    if (IsString("Off") || IsString(""))
        obj = SilKit::Config::Aggregation::Off;
    else if (IsString("On"))
        obj = SilKit::Config::Aggregation::On;
    else if (IsString("Auto"))
        obj = SilKit::Config::Aggregation::Auto;
    else
    {
        throw MakeConfigurationError("Unknown Aggregation");
    }
}

void YamlReader::Read(SilKit::Config::TimeSynchronization& obj)
{
    OptionalRead(obj.animationFactor, "AnimationFactor");
    OptionalRead(obj.enableMessageAggregation, "EnableMessageAggregation");
}

void YamlReader::Read(SilKit::Config::Experimental& obj)
{
    OptionalRead(obj.timeSynchronization, "TimeSynchronization");
    OptionalRead(obj.metrics, "Metrics");
}

void YamlReader::Read(SilKit::Config::ParticipantConfiguration& obj)
{
    OptionalRead(obj.schemaVersion, "schemaVersion"); // note lower case schemaVersion
    OptionalRead(obj.description, "Description");
    OptionalRead(obj.participantName, "ParticipantName");

    OptionalRead(obj.canControllers, "CanControllers");
    OptionalRead(obj.linControllers, "LinControllers");
    OptionalRead(obj.ethernetControllers, "EthernetControllers");
    OptionalRead_deprecated_alternative(obj.flexrayControllers, "FlexrayControllers", {"FlexRayControllers"});
    OptionalRead(obj.dataPublishers, "DataPublishers");
    OptionalRead(obj.dataSubscribers, "DataSubscribers");
    OptionalRead(obj.rpcServers, "RpcServers");
    OptionalRead(obj.rpcClients, "RpcClients");

    OptionalRead(obj.logging, "Logging");
    OptionalRead(obj.healthCheck, "HealthCheck");
    OptionalRead(obj.tracing, "Tracing");
    OptionalRead(obj.extensions, "Extensions");
    OptionalRead(obj.middleware, "Middleware");
    OptionalRead(obj.includes, "Includes");
    OptionalRead(obj.experimental, "Experimental");
}

void YamlReader::Read(SilKit::Config::HealthCheck& obj)
{
    OptionalRead(obj.softResponseTimeout, "SoftResponseTimeout");
    OptionalRead(obj.hardResponseTimeout, "HardResponseTimeout");
}
//Registry
void YamlReader::Read(SilKitRegistry::Config::v1::Experimental& obj)
{
    OptionalRead(obj.metrics, "Metrics");

    for (auto&& sink : obj.metrics.sinks)
    {
        if (sink.type == SilKit::Config::MetricsSink::Type::Remote)
        {
            throw SilKit::ConfigurationError{"SIL Kit Registry does not support remote metrics sinks"};
        }
    }
}

void YamlReader::Read(SilKitRegistry::Config::v1::RegistryConfiguration& obj)
{
    std::string schemaVersion;
    OptionalRead(schemaVersion, "SchemaVersion");
    if (!schemaVersion.empty() && schemaVersion != SilKitRegistry::Config::v1::GetSchemaVersion())
    {
        throw SilKit::ConfigurationError{"Unknown schema version '{}' found in registry configuration!"};
    }

    OptionalRead(obj.description, "Description");
    OptionalRead(obj.listenUri, "ListenUri");
    OptionalRead(obj.enableDomainSockets, "EnableDomainSockets");
    OptionalRead(obj.dashboardUri, "DashboardUri");
    OptionalRead(obj.logging, "Logging");
    OptionalRead(obj.experimental, "Experimental");

    if (obj.logging.logFromRemotes)
    {
        throw SilKit::ConfigurationError{"SIL Kit Registry does not support receiving logs from remotes"};
    }

    for (auto&& sink : obj.logging.sinks)
    {
        if (sink.type == SilKit::Config::Sink::Type::Remote)
        {
            throw SilKit::ConfigurationError{"SIL Kit Registry does not support remote logging"};
        }
    }
}

} // namespace VSilKit
