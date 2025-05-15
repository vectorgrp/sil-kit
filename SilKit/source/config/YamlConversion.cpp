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

#include "YamlConversion.hpp"
#include "SilKitYamlHelper.hpp"

#include <algorithm>
#include <sstream>
#include <chrono>
#include <type_traits>
#include <silkit/capi/Types.h>

using namespace SilKit::Config;
using namespace SilKit;

// XXXXXXXXXX RAPID YML XXXXXXXXXXXXXX

namespace std {
namespace chrono {
void write(ryml::NodeRef* node, const std::chrono::milliseconds& obj)
{
    Write(node, std::to_string(obj.count()));
}

bool read(const ryml::ConstNodeRef& node, std::chrono::milliseconds* obj)
{
    uint64_t buf;
    auto tmp = ryml::fmt::overflow_checked(buf);
    node >> tmp;
    *obj = std::chrono::milliseconds{*tmp.val};
    return true;
}

void write(ryml::NodeRef* node, const std::chrono::nanoseconds& obj)
{
    Write(node, std::to_string(obj.count()));
}

bool read(const ryml::ConstNodeRef& node, std::chrono::nanoseconds* obj)
{
    uint64_t buf;
    auto tmp = ryml::fmt::overflow_checked(buf);
    node >> tmp;
    *obj = std::chrono::nanoseconds{*tmp.val};
    return true;
}

} // namespace chrono
} // namespace std
namespace SilKit {
namespace Services {

void write(ryml::NodeRef* node, const SilKit::Services::MatchingLabel::Kind& obj)
{
    switch (obj)
    {
    case Services::MatchingLabel::Kind::Mandatory:
        Write(node, "Mandatory");
        break;
    case Services::MatchingLabel::Kind::Optional:
        Write(node, "Optional");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, SilKit::Services::MatchingLabel::Kind* obj)
{
    if (!IsScalar(node))
    {
        throw ConfigurationError("Level should be a string of Critical|Error|Warn|Info|Debug|Trace|Off.");
    }
    auto&& str = node.val();
    if (str == "Mandatory")
        *obj = SilKit::Services::MatchingLabel::Kind::Mandatory;
    else if (str == "Optional")
        *obj = SilKit::Services::MatchingLabel::Kind::Optional;
    return true;
}

void write(ryml::NodeRef* node, const SilKit::Services::MatchingLabel& obj)
{
    MakeMap(node);
    Write(node, "key", obj.key);
    Write(node, "value", obj.value);
    Write(node, "kind", obj.kind);
}

bool read(const ryml::ConstNodeRef& node, SilKit::Services::MatchingLabel* obj)
{
    OptionalRead(obj->key, node, "key");
    OptionalRead(obj->value, node, "value");
    OptionalRead(obj->kind, node, "kind");
    return true;
}

namespace Logging {
using namespace SilKit::Config;
void write (ryml::NodeRef* node,  const Services::Logging::Level& obj)
{
    switch (obj)
    {
    case Services::Logging::Level::Critical:
        Write(node, "Critical");
        break;
    case Services::Logging::Level::Error:
        Write(node, "Error");
        break;
    case Services::Logging::Level::Warn:
        Write(node, "Warn");
        break;
    case Services::Logging::Level::Info:
        Write(node, "Info");
        break;
    case Services::Logging::Level::Debug:
        Write(node, "Debug");
        break;
    case Services::Logging::Level::Trace:
        Write(node, "Trace");
        break;
    case Services::Logging::Level::Off:
        Write(node, "Off");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, Services::Logging::Level* obj)
{
    if (!IsScalar(node))
    {
        throw ConfigurationError( "Level should be a string of Critical|Error|Warn|Info|Debug|Trace|Off.");
    }
    auto&& str = node.val();
    if (str == "Critical")
        *obj = Services::Logging::Level::Critical;
    else if (str == "Error")
        *obj = Services::Logging::Level::Error;
    else if (str == "Warn")
        *obj = Services::Logging::Level::Warn;
    else if (str == "Info")
        *obj = Services::Logging::Level::Info;
    else if (str == "Debug")
        *obj = Services::Logging::Level::Debug;
    else if (str == "Trace")
        *obj = Services::Logging::Level::Trace;
    else if (str == "Off")
        *obj = Services::Logging::Level::Off;
    else
    {
        throw ConfigurationError(Format("Unknown Services::Logging::Level: {}.", str ));
    }
    return true;
}

} // namespace Logging

namespace Flexray {

void write (ryml::NodeRef* node,  const FlexrayClusterParameters& obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    MakeMap(node);
    Write(node,"gColdstartAttempts",obj.gColdstartAttempts);
    Write(node,"gCycleCountMax",obj.gCycleCountMax);
    Write(node,"gdActionPointOffset",obj.gdActionPointOffset);
    Write(node,"gdDynamicSlotIdlePhase",obj.gdDynamicSlotIdlePhase);
    Write(node,"gdMiniSlot",obj.gdMiniSlot);
    Write(node,"gdMiniSlotActionPointOffset",obj.gdMiniSlotActionPointOffset);
    Write(node,"gdStaticSlot",obj.gdStaticSlot);
    Write(node,"gdSymbolWindow",obj.gdSymbolWindow);
    Write(node,"gdSymbolWindowActionPointOffset",obj.gdSymbolWindowActionPointOffset);
    Write(node,"gdTSSTransmitter",obj.gdTSSTransmitter);
    Write(node,"gdWakeupTxActive",obj.gdWakeupTxActive);
    Write(node,"gdWakeupTxIdle",obj.gdWakeupTxIdle);
    Write(node,"gListenNoise",obj.gListenNoise);
    Write(node,"gMacroPerCycle",obj.gMacroPerCycle);
    Write(node,"gMaxWithoutClockCorrectionFatal",obj.gMaxWithoutClockCorrectionFatal);
    Write(node,"gMaxWithoutClockCorrectionPassive",obj.gMaxWithoutClockCorrectionPassive);
    Write(node,"gNumberOfMiniSlots",obj.gNumberOfMiniSlots);
    Write(node,"gNumberOfStaticSlots",obj.gNumberOfStaticSlots);
    Write(node,"gPayloadLengthStatic",obj.gPayloadLengthStatic);
    Write(node,"gSyncFrameIDCountMax",obj.gSyncFrameIDCountMax);
}

bool read(const ryml::ConstNodeRef& node, Services::Flexray::FlexrayClusterParameters* obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    Read(obj->gColdstartAttempts , node, "gColdstartAttempts");
    Read(obj->gCycleCountMax , node, "gCycleCountMax");
    Read(obj->gdActionPointOffset , node, "gdActionPointOffset");
    Read(obj->gdDynamicSlotIdlePhase , node, "gdDynamicSlotIdlePhase");
    Read(obj->gdMiniSlot , node, "gdMiniSlot");
    Read(obj->gdMiniSlotActionPointOffset , node, "gdMiniSlotActionPointOffset");
    Read(obj->gdStaticSlot , node, "gdStaticSlot");
    Read(obj->gdSymbolWindow , node, "gdSymbolWindow");
    Read(obj->gdSymbolWindowActionPointOffset , node, "gdSymbolWindowActionPointOffset");
    Read(obj->gdTSSTransmitter , node, "gdTSSTransmitter");
    Read(obj->gdWakeupTxActive , node, "gdWakeupTxActive");
    Read(obj->gdWakeupTxIdle , node, "gdWakeupTxIdle");
    Read(obj->gListenNoise , node, "gListenNoise");
    Read(obj->gMacroPerCycle , node, "gMacroPerCycle");
    Read(obj->gMaxWithoutClockCorrectionFatal , node, "gMaxWithoutClockCorrectionFatal");
    Read(obj->gMaxWithoutClockCorrectionPassive , node, "gMaxWithoutClockCorrectionPassive");
    Read(obj->gNumberOfMiniSlots , node, "gNumberOfMiniSlots");
    Read(obj->gNumberOfStaticSlots , node, "gNumberOfStaticSlots");
    Read(obj->gPayloadLengthStatic , node, "gPayloadLengthStatic");
    Read(obj->gSyncFrameIDCountMax , node, "gSyncFrameIDCountMax");
    return true;
}

void write (ryml::NodeRef* node,  const FlexrayNodeParameters& obj)
{
    MakeMap(node);
    Write(node,"pAllowHaltDueToClock", obj.pAllowHaltDueToClock);
    Write(node,"pAllowPassiveToActive", obj.pAllowPassiveToActive);
    Write(node,"pClusterDriftDamping", obj.pClusterDriftDamping);
    Write(node,"pdAcceptedStartupRange", obj.pdAcceptedStartupRange);
    Write(node,"pdListenTimeout", obj.pdListenTimeout);
    Write(node,"pKeySlotId", obj.pKeySlotId);
    Write(node,"pKeySlotOnlyEnabled", obj.pKeySlotOnlyEnabled);
    Write(node,"pKeySlotUsedForStartup", obj.pKeySlotUsedForStartup);
    Write(node,"pKeySlotUsedForSync", obj.pKeySlotUsedForSync);
    Write(node,"pLatestTx", obj.pLatestTx);
    Write(node,"pMacroInitialOffsetA", obj.pMacroInitialOffsetA);
    Write(node,"pMacroInitialOffsetB", obj.pMacroInitialOffsetB);
    Write(node,"pMicroInitialOffsetA", obj.pMicroInitialOffsetA);
    Write(node,"pMicroInitialOffsetB", obj.pMicroInitialOffsetB);
    Write(node,"pMicroPerCycle", obj.pMicroPerCycle);
    Write(node,"pOffsetCorrectionOut", obj.pOffsetCorrectionOut);
    Write(node,"pOffsetCorrectionStart", obj.pOffsetCorrectionStart);
    Write(node,"pRateCorrectionOut", obj.pRateCorrectionOut);
    Write(node,"pWakeupPattern", obj.pWakeupPattern);
    Write(node,"pSamplesPerMicrotick", obj.pSamplesPerMicrotick);
    Write(node,"pWakeupChannel", obj.pWakeupChannel);
    Write(node,"pdMicrotick", obj.pdMicrotick);
    Write(node,"pChannels", obj.pChannels);
}

bool read(const ryml::ConstNodeRef& node, Services::Flexray::FlexrayNodeParameters* obj)
{
    Read(obj->pAllowHaltDueToClock,node, "pAllowHaltDueToClock");
    Read(obj->pAllowPassiveToActive,node, "pAllowPassiveToActive");
    Read(obj->pClusterDriftDamping,node, "pClusterDriftDamping");
    Read(obj->pdAcceptedStartupRange,node, "pdAcceptedStartupRange");
    Read(obj->pdListenTimeout,node, "pdListenTimeout");
    Read(obj->pKeySlotId,node, "pKeySlotId");
    Read(obj->pKeySlotOnlyEnabled,node, "pKeySlotOnlyEnabled");
    Read(obj->pKeySlotUsedForStartup,node, "pKeySlotUsedForStartup");
    Read(obj->pKeySlotUsedForSync,node, "pKeySlotUsedForSync");
    Read(obj->pLatestTx,node, "pLatestTx");
    Read(obj->pMacroInitialOffsetA,node, "pMacroInitialOffsetA");
    Read(obj->pMacroInitialOffsetB,node, "pMacroInitialOffsetB");
    Read(obj->pMicroInitialOffsetA,node, "pMicroInitialOffsetA");
    Read(obj->pMicroInitialOffsetB,node, "pMicroInitialOffsetB");
    Read(obj->pMicroPerCycle,node, "pMicroPerCycle");
    Read(obj->pOffsetCorrectionOut,node, "pOffsetCorrectionOut");
    Read(obj->pOffsetCorrectionStart,node, "pOffsetCorrectionStart");
    Read(obj->pRateCorrectionOut,node, "pRateCorrectionOut");
    Read(obj->pWakeupPattern,node, "pWakeupPattern");
    Read(obj->pSamplesPerMicrotick,node, "pSamplesPerMicrotick");
    Read(obj->pWakeupChannel, node, "pWakeupChannel");
    Read(obj->pdMicrotick, node, "pdMicrotick");
    Read(obj->pChannels, node, "pChannels");
    return true;
}

void write (ryml::NodeRef* node,  const FlexrayTxBufferConfig& obj)
{
    MakeMap(node);
    Write(node,"channels",obj.channels);
    Write(node,"slotId", obj.slotId);
    Write(node,"offset", obj.offset);
    Write(node,"repetition", obj.repetition);
    Write(node,"PPindicator", obj.hasPayloadPreambleIndicator);
    Write(node,"headerCrc", obj.headerCrc);
    Write(node,"transmissionMode", obj.transmissionMode);
}

bool read(const ryml::ConstNodeRef& node, Services::Flexray::FlexrayTxBufferConfig* obj)
{
    Read(obj->channels, node, "channels");
    Read(obj->slotId, node, "slotId");
    Read(obj->offset, node, "offset");
    Read(obj->repetition, node, "repetition");
    Read(obj->hasPayloadPreambleIndicator, node, "PPindicator");
    Read(obj->headerCrc, node, "headerCrc");
    Read(obj->transmissionMode, node, "transmissionMode");
    return true;
}

void write (ryml::NodeRef* node,  const FlexrayChannel& obj)
{
    switch (obj)
    {
    case Services::Flexray::FlexrayChannel::A:
        Write(node, "A");
        break;
    case Services::Flexray::FlexrayChannel::B:
        Write(node, "B");
        break;
    case Services::Flexray::FlexrayChannel::AB:
        Write(node, "AB");
        break;
    case Services::Flexray::FlexrayChannel::None:
        Write(node, "None");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, Services::Flexray::FlexrayChannel* obj)
{
    auto&& str = node.val();
    if (str == "A")
        *obj = Services::Flexray::FlexrayChannel::A;
    else if (str == "B")
        *obj = Services::Flexray::FlexrayChannel::B;
    else if (str == "AB")
        *obj = Services::Flexray::FlexrayChannel::AB;
    else if (str == "None" || str == "")
        *obj = Services::Flexray::FlexrayChannel::None;
    else
    {
        throw ConfigurationError(Format("Unknown Services::Flexray::FlexrayChannel: {}.",str));
    }
    return true;
}

void write (ryml::NodeRef* node,  const FlexrayClockPeriod& obj)
{
    switch (obj)
    {
    case Services::Flexray::FlexrayClockPeriod::T12_5NS:
        Write(node, "12.5ns");
        break;
    case Services::Flexray::FlexrayClockPeriod::T25NS:
        Write(node, "25ns");
        break;
    case Services::Flexray::FlexrayClockPeriod::T50NS:
        Write(node, "50ns");
        break;
    default:
        throw ConfigurationError("Unknown Services::Flexray::FlexrayClockPeriod");
    }
}

bool read(const ryml::ConstNodeRef& node, FlexrayClockPeriod* obj)
{
    auto&& str = node.val();
    if (str == "12.5ns")
        *obj = Services::Flexray::FlexrayClockPeriod::T12_5NS;
    else if (str == "25ns")
        *obj = Services::Flexray::FlexrayClockPeriod::T25NS;
    else if (str == "50ns")
        *obj = Services::Flexray::FlexrayClockPeriod::T50NS;
    else
    {
        throw ConfigurationError(Format("Unknown Services::Flexray::FlexrayClockPeriod: {}.", str));
    }
    return true;
}

void write (ryml::NodeRef* node,  const FlexrayTransmissionMode& obj)
{
    switch (obj)
    {
    case Services::Flexray::FlexrayTransmissionMode::Continuous:
        Write(node , "Continuous");
        break;
    case Services::Flexray::FlexrayTransmissionMode::SingleShot:
        Write(node , "SingleShot");
        break;
    default:
        throw ConfigurationError("Unknown FlexrayTransmissionMode");
    }
}

bool read(const ryml::ConstNodeRef& node, FlexrayTransmissionMode* obj)
{
    auto&& str = node.val();
    if (str == "Continuous")
        *obj = Services::Flexray::FlexrayTransmissionMode::Continuous;
    else if (str == "SingleShot")
        *obj = Services::Flexray::FlexrayTransmissionMode::SingleShot;
    else
    {
        throw ConfigurationError(Format("Unknown Services::Flexray::FlexrayTransmissionMode: {}.", str));
    }
    return true;
}
} // namespace Flexray

} // namespace Services
namespace Config {
inline namespace v1 {

// local utility
void OptionalWrite(const Replay& value, ryml::NodeRef* node, const std::string& name)
{
    if (value.useTraceSource.size() > 0)
    {
        node->append_child() << ryml::key(name) << value;
    }
}


void write(ryml::NodeRef* node, const Sink::Type& obj)
{
    switch (obj)
    {
    case Sink::Type::Remote:
        Write(node, "Remote");
        break;
    case Sink::Type::Stdout:
        Write(node, "Stdout");
        break;
    case Sink::Type::File:
        Write(node, "File");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, Sink::Type* obj)
{
    if (!IsScalar(node))
    {
        throw ConfigurationError("Sink::Type should be a string of Remote|Stdout|File.");
    }

    auto&& str = node.val();
    if (str == "Remote" || str == "")
    {
        *obj = Sink::Type::Remote;
    }
    else if (str == "Stdout")
    {
        *obj = Sink::Type::Stdout;
    }
    else if (str == "File")
    {
        *obj = Sink::Type::File;
    }
    else
    {
        throw ConfigurationError(Format("Unknown Sink::Type: {}.", str));
    }
    return true;
}

void write(ryml::NodeRef* node, const Sink::Format& obj)
{
    switch (obj)
    {
    case Sink::Format::Simple:
        Write(node, "Simple");
        break;
    case Sink::Format::Json:
        Write(node, "Json");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, Sink::Format* obj)
{
    if (!IsScalar(node))
    {
        throw ConfigurationError("Sink::Format should be a string of Simple|Json.");
    }
    auto&& str = node.val();
    if (str == "Simple" || str == "")
    {
        *obj = Sink::Format::Simple;
    }
    else if (str == "Json")
    {
        *obj = Sink::Format::Json;
    }
    else
    {
        throw ConfigurationError(Format("Unknown Sink::Format: {}.", str));
    }
    return true;
}

void write(ryml::NodeRef* node, const Sink& obj)
{
    static const Sink defaultSink{};
    // ParticipantConfiguration.schema.json: Type is required:
    MakeMap(node);
    Write(node, "Type", obj.type);
    NonDefaultWrite(obj.level, node, "Level", defaultSink.level);
    NonDefaultWrite(obj.logName, node, "LogName", defaultSink.logName);
}

bool read(const ryml::ConstNodeRef& node, Sink* obj)
{
    OptionalRead(obj->type, node, "Type");
    OptionalRead(obj->level, node, "Level");
    OptionalRead(obj->format, node, "Format");

    if (obj->type == Sink::Type::File)
    {
        if (!IsValidChild(node, "LogName"))
        {
            throw ConfigurationError("Sink of type Sink::Type::File requires a LogName");
        }
        Read(obj->logName, node, "LogName");
    }
    return true;
}

void write(ryml::NodeRef* node, const Logging& obj)
{
    static const Logging defaultLogger{};
    MakeMap(node);
    NonDefaultWrite(obj.logFromRemotes, node, "LogFromRemotes", defaultLogger.logFromRemotes);
    NonDefaultWrite(obj.flushLevel, node, "FlushLevel", defaultLogger.flushLevel);
    // ParticipantConfiguration.schema.json: this is a required property:
    Write(node, "Sinks", obj.sinks);
}

bool read(const ryml::ConstNodeRef& node, Logging* obj)
{
    OptionalRead(obj->logFromRemotes, node, "LogFromRemotes");
    OptionalRead(obj->flushLevel, node, "FlushLevel");
    OptionalRead(obj->sinks, node, "Sinks");
    return true;
}

// Metrics
void write(ryml::NodeRef* node, const MetricsSink::Type& obj)
{
    switch (obj)
    {
    case MetricsSink::Type::Undefined:
        Write(node, "Undefined");
        break;
    case MetricsSink::Type::JsonFile:
        Write(node, "JsonFile");
        break;
    case MetricsSink::Type::Remote:
        Write(node, "Remote");
        break;
    default:
        throw ConfigurationError{"Unknown MetricsSink Type"};
    }
}

bool read(const ryml::ConstNodeRef& node, MetricsSink::Type* obj)
{
    auto&& str = node.val();
    if (str == "Undefined" || str == "")
    {
        *obj = MetricsSink::Type::Undefined;
    }
    else if (str == "JsonFile")
    {
        *obj = MetricsSink::Type::JsonFile;
    }
    else if (str == "Remote")
    {
        *obj = MetricsSink::Type::Remote;
    }
    else
    {
        throw ConfigurationError{Format("Unknown MetricsSink::Type: {}.", str)};
    }
    return true;
}

void write(ryml::NodeRef* node, const MetricsSink& obj)
{
    MakeMap(node);
    Write(node, "Type", obj.type);
    if (!obj.name.empty())
    {
        Write(node, "Name", obj.name);
    }
}

bool read(const ryml::ConstNodeRef& node, MetricsSink* obj)
{
    Read(obj->type, node, "Type");
    OptionalRead(obj->name, node, "Name");
    return true;
}

void write(ryml::NodeRef* node, const Metrics& obj)
{
    MakeMap(node);
    OptionalWrite(obj.sinks, node, "Sinks");
    if (obj.collectFromRemote)
    {
        Write(node, "CollectFromRemote", obj.collectFromRemote);
    }
}

bool read(const ryml::ConstNodeRef& node, Metrics* obj)
{
    OptionalRead(obj->sinks, node, "Sinks");
    OptionalRead(obj->collectFromRemote, node, "CollectFromRemote");

    if (obj->collectFromRemote)
    {
        for (auto&& sink : obj->sinks)
        {
            if (sink.type == SilKit::Config::MetricsSink::Type::Remote)
            {
                throw SilKit::ConfigurationError{
                    "Metrics collectFromRemote is enabled while having a Remote MetricsSink active"};  
            }
        }
    }
    return true;
}

void write(ryml::NodeRef* node, const MdfChannel& obj)
{
    MakeMap(node);
    OptionalWrite(obj.channelName, node, "ChannelName");
    OptionalWrite(obj.channelPath, node, "ChannelPath");
    OptionalWrite(obj.channelSource, node, "ChannelSource");
    OptionalWrite(obj.groupName, node, "GroupName");
    OptionalWrite(obj.groupPath, node, "GroupPath");
    OptionalWrite(obj.groupSource, node, "GroupSource");
}

bool read(const ryml::ConstNodeRef& node, MdfChannel* obj)
{
    if (!IsMap(node))
    {
        throw ConfigurationError("MdfChannel should be a Map");
    }
    OptionalRead(obj->channelName, node, "ChannelName");
    OptionalRead(obj->channelPath, node, "ChannelPath");
    OptionalRead(obj->channelSource, node, "ChannelSource");
    OptionalRead(obj->groupName, node, "GroupName");
    OptionalRead(obj->groupPath, node, "GroupPath");
    OptionalRead(obj->groupSource, node, "GroupSource");
    return true;
}

void write(ryml::NodeRef* node, const Replay& obj)
{
    static const Replay defaultObject{};
    MakeMap(node);
    Write(node, "UseTraceSource", obj.useTraceSource);
    NonDefaultWrite(obj.direction, node, "Direction", defaultObject.direction);
    NonDefaultWrite(obj.mdfChannel, node, "MdfChannel", defaultObject.mdfChannel);
}

bool read(const ryml::ConstNodeRef& node, Replay* obj)
{
    Read(obj->useTraceSource, node, "UseTraceSource");
    OptionalRead(obj->direction, node, "Direction");
    OptionalRead(obj->mdfChannel, node, "MdfChannel");
    return true;
}

void write(ryml::NodeRef* node, const Replay::Direction& obj)
{
    switch (obj)
    {
    case Replay::Direction::Send:
        Write(node, "Send");
        break;
    case Replay::Direction::Receive:
        Write(node, "Receive");
        break;
    case Replay::Direction::Both:
        Write(node, "Both");
        break;
    case Replay::Direction::Undefined:
        Write(node, "Undefined");
        break;
    }
}

bool read(const ryml::ConstNodeRef& node, Replay::Direction* obj)
{
    auto&& str = node.val();
    if (str == "Undefined" || str == "")
        *obj = Replay::Direction::Undefined;
    else if (str == "Send")
        *obj = Replay::Direction::Send;
    else if (str == "Receive")
        *obj = Replay::Direction::Receive;
    else if (str == "Both")
        *obj = Replay::Direction::Both;
    else
    {
        throw ConfigurationError(Format("Unknown Replay::Direction: {}.", str ));
    }
    return true;
}

void write(ryml::NodeRef* node, const CanController& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}

bool read(const ryml::ConstNodeRef& node, CanController* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->network, node, "Network");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const LinController& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}

bool read(const ryml::ConstNodeRef& node, LinController* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->network, node, "Network");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const EthernetController& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}

bool read(const ryml::ConstNodeRef& node, EthernetController* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->network, node, "Network");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const FlexrayController& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.clusterParameters, node, "ClusterParameters");
    OptionalWrite(obj.nodeParameters, node, "NodeParameters");
    OptionalWrite(obj.txBufferConfigurations, node, "TxBufferConfigurations");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}
bool read(const ryml::ConstNodeRef& node, FlexrayController* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->network, node, "Network");
    OptionalRead(obj->clusterParameters, node, "ClusterParameters");
    OptionalRead(obj->nodeParameters, node, "NodeParameters");
    OptionalRead(obj->txBufferConfigurations, node, "TxBufferConfigurations");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const Label::Kind& obj)
{
    switch (obj)
    {
    case Label::Kind::Mandatory:
        Write(node, "Mandatory");
        break;
    case Label::Kind::Optional:
        Write(node, "Optional");
        break;
    default:
        throw ConfigurationError{"Unknown Label::Kind"};
    }
}

bool read(const ryml::ConstNodeRef& node, Label::Kind* obj)
{
    auto&& str = node.val();
    if (str == "Mandatory")
        *obj = Label::Kind::Mandatory;
    else if (str == "Optional")
        *obj = Label::Kind::Optional;
    else
    {
        throw ConfigurationError(Format("Unknown Label::Kind: {}.", str));
    }

    return true;
}

void write(ryml::NodeRef* node, const Label& obj)
{
    MakeMap(node);
    Write(node, "Key", obj.key);
    Write(node,"Value", obj.value);
    Write(node,"Kind", obj.kind);
}
bool read(const ryml::ConstNodeRef& node, Label* obj)
{
    OptionalRead(obj->key, node, "Key");
    OptionalRead(obj->value, node, "Value");
    OptionalRead(obj->kind, node, "Kind");
    return true;
}

void write(ryml::NodeRef* node, const DataPublisher& obj)
{
    MakeMap(node);
    Write(node,"Name",obj.name);
    OptionalWrite(obj.topic, node, "Topic");
    OptionalWrite(obj.labels, node, "Labels");
    //OptionalWrite(obj.history, node, "History");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}

bool read(const ryml::ConstNodeRef& node, DataPublisher* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->topic, node, "Topic");
    OptionalRead(obj->labels, node, "Labels");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const DataSubscriber& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.topic, node, "Topic");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}
bool read(const ryml::ConstNodeRef& node, DataSubscriber* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead(obj->topic, node, "Topic");
    OptionalRead(obj->labels, node, "Labels");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const RpcServer& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.functionName, node, "FunctionName");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}

bool read(const ryml::ConstNodeRef& node, RpcServer* obj)
{
    Read(obj->name, node, "Name");

    OptionalRead_deprecated_alternative(obj->functionName, node, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj->labels, node, "Labels");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const RpcClient& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    OptionalWrite(obj.functionName, node, "Channel");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
}
bool read(const ryml::ConstNodeRef& node, RpcClient* obj)
{
    Read(obj->name, node, "Name");
    OptionalRead_deprecated_alternative(obj->functionName, node, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj->labels, node, "Labels");
    OptionalRead(obj->useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj->replay, node, "Replay");
    return true;
}

void write(ryml::NodeRef* node, const Tracing& obj)
{
    MakeMap(node);
    OptionalWrite(obj.traceSinks, node, "TraceSinks");
    OptionalWrite(obj.traceSources, node, "TraceSources");
}
bool read(const ryml::ConstNodeRef& node, Tracing* obj)
{
    OptionalRead(obj->traceSinks, node, "TraceSinks");
    OptionalRead(obj->traceSources, node, "TraceSources");
    return true;
}

void write(ryml::NodeRef* node, const TraceSink& obj)
{
    MakeMap(node);
    Write(node, "Name", obj.name);
    Write(node, "Type", obj.type);
    Write(node, "OutputPath", obj.outputPath);
}

bool read(const ryml::ConstNodeRef& node, TraceSink* obj)
{
    Read(obj->name, node,"Name");
    Read(obj->type, node,"Type");
    Read(obj->outputPath, node, "OutputPath");
    return true;
}

void write(ryml::NodeRef* node, const TraceSink::Type& obj)
{
    switch (obj)
    {
    case TraceSink::Type::Undefined:
        Write(node , "Undefined");
        break;
    case TraceSink::Type::Mdf4File:
        Write(node , "Mdf4File");
        break;
    case TraceSink::Type::PcapFile:
        Write(node , "PcapFile");
        break;
    case TraceSink::Type::PcapPipe:
        Write(node , "PcapPipe");
        break;
    default:
        throw ConfigurationError{"Unknown TraceSink Type"};
    }
}

bool read(const ryml::ConstNodeRef& node, TraceSink::Type* obj)
{
    auto&& str = node.val();
    if (str == "Undefined" || str == "")
        *obj = TraceSink::Type::Undefined;
    else if (str == "Mdf4File")
        *obj = TraceSink::Type::Mdf4File;
    else if (str == "PcapFile")
        *obj = TraceSink::Type::PcapFile;
    else if (str == "PcapPipe")
        *obj = TraceSink::Type::PcapPipe;
    else
    {
        throw ConfigurationError(Format("Unknown TraceSink::Type: {}.", str));
    }
    return true;
}

void write(ryml::NodeRef* node, const TraceSource& obj)
{
    MakeMap(node);
    Write(node,"Name", obj.name);
    Write(node,"Type", obj.type);
    Write(node,"InputPath", obj.inputPath);
}

bool read(const ryml::ConstNodeRef& node, TraceSource* obj)
{
    Read(obj->name, node, "Name");
    Read(obj->type, node,"Type");
    Read(obj->inputPath,node,"InputPath");
    return true;
}

void write(ryml::NodeRef* node, const TraceSource::Type& obj)
{
    switch (obj)
    {
    case TraceSource::Type::Undefined:
        Write(node ,"Undefined");
        break;
    case TraceSource::Type::Mdf4File:
        Write(node, "Mdf4File");
        break;
    case TraceSource::Type::PcapFile:
        Write(node, "PcapFile");
        break;
    default:
        throw ConfigurationError{"Unknown TraceSource Type"};
    }
}

bool read(const ryml::ConstNodeRef& node, TraceSource::Type* obj)
{
    auto&& str = node.val();
    if (str == "Undefined" || str == "")
        *obj = TraceSource::Type::Undefined;
    else if (str == "Mdf4File")
        *obj = TraceSource::Type::Mdf4File;
    else if (str == "PcapFile")
        *obj = TraceSource::Type::PcapFile;
    else
    {
        throw ConfigurationError(Format("Unknown TraceSource::Type: {}.", str));
    }
    return true;
}

void write(ryml::NodeRef* node, const Extensions& obj)
{
    static const Extensions defaultObj{};
    MakeMap(node);
    NonDefaultWrite(obj.searchPathHints, node, "SearchPathHints", defaultObj.searchPathHints);
}
bool read(const ryml::ConstNodeRef& node, Extensions* obj)
{
    OptionalRead(obj->searchPathHints, node, "SearchPathHints");
    return true;
}


void write(ryml::NodeRef* node, const Middleware& obj)
{
    static const Middleware defaultObj;
    MakeMap(node);
    NonDefaultWrite(obj.registryUri, node, "RegistryUri", defaultObj.registryUri);
    NonDefaultWrite(obj.connectAttempts, node, "ConnectAttempts", defaultObj.connectAttempts);
    NonDefaultWrite(obj.tcpNoDelay, node, "TcpNoDelay", defaultObj.tcpNoDelay);
    NonDefaultWrite(obj.tcpQuickAck, node, "TcpQuickAck", defaultObj.tcpQuickAck);
    NonDefaultWrite(obj.tcpReceiveBufferSize, node, "TcpReceiveBufferSize", defaultObj.tcpReceiveBufferSize);
    NonDefaultWrite(obj.tcpSendBufferSize, node, "TcpSendBufferSize", defaultObj.tcpSendBufferSize);
    NonDefaultWrite(obj.enableDomainSockets, node, "EnableDomainSockets", defaultObj.enableDomainSockets);
    NonDefaultWrite(obj.acceptorUris, node, "acceptorUris", defaultObj.acceptorUris);
    NonDefaultWrite(obj.registryAsFallbackProxy, node, "RegistryAsFallbackProxy",
                       defaultObj.registryAsFallbackProxy);
    NonDefaultWrite(obj.experimentalRemoteParticipantConnection, node, "ExperimentalRemoteParticipantConnection",
                       defaultObj.experimentalRemoteParticipantConnection);
    NonDefaultWrite(obj.connectTimeoutSeconds, node, "ConnectTimeoutSeconds", defaultObj.connectTimeoutSeconds);
}

bool read(const ryml::ConstNodeRef& node, Middleware* obj)
{
    OptionalRead(obj->registryUri, node, "RegistryUri");
    OptionalRead(obj->connectAttempts, node, "ConnectAttempts");
    OptionalRead(obj->tcpNoDelay, node, "TcpNoDelay");
    OptionalRead(obj->tcpQuickAck, node, "TcpQuickAck");
    OptionalRead(obj->tcpReceiveBufferSize, node, "TcpReceiveBufferSize");
    OptionalRead(obj->tcpSendBufferSize, node, "TcpSendBufferSize");
    OptionalRead(obj->enableDomainSockets, node, "EnableDomainSockets");
    OptionalRead(obj->acceptorUris, node, "AcceptorUris");
    OptionalRead(obj->registryAsFallbackProxy, node, "RegistryAsFallbackProxy");
    OptionalRead(obj->experimentalRemoteParticipantConnection, node, "ExperimentalRemoteParticipantConnection");
    OptionalRead(obj->connectTimeoutSeconds, node, "ConnectTimeoutSeconds");
    return true;
}

void write(ryml::NodeRef* node, const Includes& obj)
{
    MakeMap(node);
    OptionalWrite(obj.files, node, "Files"); 
    OptionalWrite(obj.searchPathHints, node, "SearchPathHints"); 
}

bool read(const ryml::ConstNodeRef& node, Includes* obj)
{
    OptionalRead(obj->files, node, "Files");
    OptionalRead(obj->searchPathHints, node, "SearchPathHints");
    return true;
}
void write(ryml::NodeRef* node, const Aggregation& obj)
{
    switch (obj)
    {
    case Aggregation::Off:
        Write(node, "Off");
        break;
    case Aggregation::On:
        Write(node , "On");
        break;
    case Aggregation::Auto:
        Write(node , "Auto");
        break;
    default:
        throw ConfigurationError{"Unknown Aggregation Type"};
    }
}
bool read(const ryml::ConstNodeRef& node, Aggregation* obj)
{
    auto&& str = node.val();
    if (str == "Off" || str == "")
        *obj = Aggregation::Off;
    else if (str == "On")
        *obj = Aggregation::On;
    else if (str == "Auto")
        *obj = Aggregation::Auto;
    else
    {
        throw ConfigurationError(Format("Unknown Aggregation: {}.", str));
    }
    return true;
}

void write(ryml::NodeRef* node, const TimeSynchronization& obj)
{
    static const TimeSynchronization defaultObj;
    MakeMap(node);
    NonDefaultWrite(obj.animationFactor, node, "AnimationFactor", defaultObj.animationFactor);
    NonDefaultWrite(obj.enableMessageAggregation, node, "EnableMessageAggregation",
                       defaultObj.enableMessageAggregation);
}

bool read(const ryml::ConstNodeRef& node, TimeSynchronization* obj)
{
    OptionalRead(obj->animationFactor, node, "AnimationFactor");
    OptionalRead(obj->enableMessageAggregation, node, "EnableMessageAggregation");
    return true;
}

void write(ryml::NodeRef* node, const Experimental& obj)
{
    static const Experimental defaultObj{};

    MakeMap(node);
    NonDefaultWrite(obj.timeSynchronization, node, "TimeSynchronization", defaultObj.timeSynchronization);
    NonDefaultWrite(obj.metrics, node, "Metrics", defaultObj.metrics);
}

bool read(const ryml::ConstNodeRef& node, Experimental* obj)
{
    OptionalRead(obj->timeSynchronization, node, "TimeSynchronization");
    OptionalRead(obj->metrics, node, "Metrics");
    return true;
}

void write(ryml::NodeRef* node, const ParticipantConfiguration& obj)
{
    static const ParticipantConfiguration defaultObj{};
    MakeMap(node);
    Write(node, "SchemaVersion", obj.schemaVersion);
    OptionalWrite(obj.description, node, "Description");
    OptionalWrite(obj.participantName, node, "ParticipantName");
    OptionalWrite(obj.canControllers, node, "CanControllers");
    OptionalWrite(obj.linControllers, node, "LinControllers");
    OptionalWrite(obj.ethernetControllers, node, "EthernetControllers");
    OptionalWrite(obj.flexrayControllers, node, "FlexrayControllers");
    OptionalWrite(obj.dataPublishers, node, "DataPublishers");
    OptionalWrite(obj.dataSubscribers, node, "DataSubscribers");
    OptionalWrite(obj.rpcServers, node, "RpcServers");
    OptionalWrite(obj.rpcClients, node, "RpcClients");

    NonDefaultWrite(obj.logging, node, "Logging", defaultObj.logging);
    NonDefaultWrite(obj.healthCheck, node, "Extensions", defaultObj.healthCheck);
    NonDefaultWrite(obj.tracing, node, "Extensions", defaultObj.tracing);
    NonDefaultWrite(obj.extensions, node, "Extensions", defaultObj.extensions);
    NonDefaultWrite(obj.middleware, node, "Middleware", defaultObj.middleware);
    NonDefaultWrite(obj.includes, node, "Includes", defaultObj.includes);
    NonDefaultWrite(obj.experimental, node, "Experimental", defaultObj.experimental);
}
bool read(const ryml::ConstNodeRef& node, ParticipantConfiguration* obj)
{
    OptionalRead(obj->schemaVersion, node, "schemaVersion"); // note lower case schemaVersion
    OptionalRead(obj->description, node, "Description");
    OptionalRead(obj->participantName, node, "ParticipantName");

    OptionalRead(obj->canControllers, node, "CanControllers");
    OptionalRead(obj->linControllers, node, "LinControllers");
    OptionalRead(obj->ethernetControllers, node, "EthernetControllers");
    OptionalRead_deprecated_alternative(obj->flexrayControllers, node, "FlexrayControllers", {"FlexRayControllers"});
    OptionalRead(obj->dataPublishers, node, "DataPublishers");
    OptionalRead(obj->dataSubscribers, node, "DataSubscribers");
    OptionalRead(obj->rpcServers, node, "RpcServers");
    OptionalRead(obj->rpcClients, node, "RpcClients");

    OptionalRead(obj->logging, node, "Logging");
    OptionalRead(obj->healthCheck, node, "HealthCheck");
    OptionalRead(obj->tracing, node, "Tracing");
    OptionalRead(obj->extensions, node, "Extensions");
    OptionalRead(obj->middleware, node, "Middleware");
    OptionalRead(obj->includes, node, "Includes");
    OptionalRead(obj->experimental, node, "Experimental");

    return true;
}

void write(ryml::NodeRef* node, const HealthCheck& obj)
{
    MakeMap(node);
    OptionalWrite(obj.softResponseTimeout, node, "SoftResponseTimeout");
    OptionalWrite(obj.hardResponseTimeout, node, "HardResponseTimeout");
}

bool read(const ryml::ConstNodeRef& node, HealthCheck* obj)
{
    OptionalRead(obj->softResponseTimeout, node, "SoftResponseTimeout");
    OptionalRead(obj->hardResponseTimeout, node, "HardResponseTimeout");
    return true;
}

} // namespace v1
} //end namespace Config
} //end namespace SilKit

// XXXXXXXXXX END RAPID YML XXXXXXXXXXXXXX

