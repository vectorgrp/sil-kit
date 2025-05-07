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

#include <algorithm>
#include <sstream>
#include <chrono>
#include <type_traits>
#include <silkit/capi/Types.h>

using namespace SilKit::Config;
using namespace SilKit;

// Local utilities
namespace {

void OptionalWrite(const Replay& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.useTraceSource.size() > 0)
    {
        node[fieldName] = value;
    }
}

} // anonymous namespace

// YAML type conversion helpers for ParticipantConfiguration data types
namespace YAML {

template <>
Node Converter::encode(const std::chrono::milliseconds& obj)
{
    Node node;
    node = obj.count();
    return node;
}
template <>
bool Converter::decode(const Node& node, std::chrono::milliseconds& obj)
{
    obj = std::chrono::milliseconds{parse_as<uint64_t>(node)};
    return true;
}

template <>
Node Converter::encode(const std::chrono::nanoseconds& obj)
{
    Node node;
    node = obj.count();
    return node;
}
template <>
bool Converter::decode(const Node& node, std::chrono::nanoseconds& obj)
{
    obj = std::chrono::nanoseconds{parse_as<uint64_t>(node)};
    return true;
}

template <>
Node Converter::encode(const Logging& obj)
{
    Node node;
    static const Logging defaultLogger{};

    NonDefaultWrite(obj.logFromRemotes, node, "LogFromRemotes", defaultLogger.logFromRemotes);
    NonDefaultWrite(obj.flushLevel, node, "FlushLevel", defaultLogger.flushLevel);
    // ParticipantConfiguration.schema.json: this is a required property:
    node["Sinks"] = obj.sinks;

    return node;
}
template <>
bool Converter::decode(const Node& node, Logging& obj)
{
    OptionalRead(obj.logFromRemotes, node, "LogFromRemotes");
    OptionalRead(obj.flushLevel, node, "FlushLevel");
    OptionalRead(obj.sinks, node, "Sinks");
    return true;
}

template <>
Node Converter::encode(const Sink& obj)
{
    static const Sink defaultSink{};
    Node node;
    // ParticipantConfiguration.schema.json: Type is required:
    node["Type"] = obj.type;
    NonDefaultWrite(obj.level, node, "Level", defaultSink.level);
    NonDefaultWrite(obj.logName, node, "LogName", defaultSink.logName);
    return node;
}
template <>
bool Converter::decode(const Node& node, Sink& obj)
{
    OptionalRead(obj.type, node, "Type");
    OptionalRead(obj.level, node, "Level");
    OptionalRead(obj.format, node, "Format");

    if (obj.type == Sink::Type::File)
    {
        if (!node["LogName"])
        {
            throw ConversionError(node, "Sink of type Sink::Type::File requires a LogName");
        }
        obj.logName = parse_as<std::string>(node["LogName"]);
    }
    return true;
}


template <>
Node Converter::encode(const Sink::Format& obj)
{
    Node node;
    switch (obj)
    {
    case Sink::Format::Simple:
        node = "Simple";
        break;
    case Sink::Format::Json:
        node = "Json";
        break;
    }
    return node;
}

template <>
bool Converter::decode(const Node& node, Sink::Format& obj)
{
    if (!node.IsScalar())
    {
        throw ConversionError(node, "Sink::Format should be a string of Simple|Json.");
    }
    auto&& str = parse_as<std::string>(node);
    if (str == "Simple" || str == "")
    {
        obj = Sink::Format::Simple;
    }
    else if (str == "Json")
    {
        obj = Sink::Format::Json;
    }
    else
    {
        throw ConversionError(node, "Unknown Sink::Format: " + str + ".");
    }
    return true;
}


template <>
Node Converter::encode(const Sink::Type& obj)
{
    Node node;
    switch (obj)
    {
    case Sink::Type::Remote:
        node = "Remote";
        break;
    case Sink::Type::Stdout:
        node = "Stdout";
        break;
    case Sink::Type::File:
        node = "File";
        break;
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Sink::Type& obj)
{
    if (!node.IsScalar())
    {
        throw ConversionError(node, "Sink::Type should be a string of Remote|Stdout|File.");
    }
    auto&& str = parse_as<std::string>(node);
    if (str == "Remote" || str == "")
    {
        obj = Sink::Type::Remote;
    }
    else if (str == "Stdout")
    {
        obj = Sink::Type::Stdout;
    }
    else if (str == "File")
    {
        obj = Sink::Type::File;
    }
    else
    {
        throw ConversionError(node, "Unknown Sink::Type: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const Services::Logging::Level& obj)
{
    Node node;
    switch (obj)
    {
    case Services::Logging::Level::Critical:
        node = "Critical";
        break;
    case Services::Logging::Level::Error:
        node = "Error";
        break;
    case Services::Logging::Level::Warn:
        node = "Warn";
        break;
    case Services::Logging::Level::Info:
        node = "Info";
        break;
    case Services::Logging::Level::Debug:
        node = "Debug";
        break;
    case Services::Logging::Level::Trace:
        node = "Trace";
        break;
    case Services::Logging::Level::Off:
        node = "Off";
        break;
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Logging::Level& obj)
{
    if (!node.IsScalar())
    {
        throw ConversionError(node, "Level should be a string of Critical|Error|Warn|Info|Debug|Trace|Off.");
    }
    auto&& str = parse_as<std::string>(node);
    if (str == "Critical")
        obj = Services::Logging::Level::Critical;
    else if (str == "Error")
        obj = Services::Logging::Level::Error;
    else if (str == "Warn")
        obj = Services::Logging::Level::Warn;
    else if (str == "Info")
        obj = Services::Logging::Level::Info;
    else if (str == "Debug")
        obj = Services::Logging::Level::Debug;
    else if (str == "Trace")
        obj = Services::Logging::Level::Trace;
    else if (str == "Off")
        obj = Services::Logging::Level::Off;
    else
    {
        throw ConversionError(node, "Unknown Services::Logging::Level: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const MdfChannel& obj)
{
    Node node;
    OptionalWrite(obj.channelName, node, "ChannelName");
    OptionalWrite(obj.channelPath, node, "ChannelPath");
    OptionalWrite(obj.channelSource, node, "ChannelSource");
    OptionalWrite(obj.groupName, node, "GroupName");
    OptionalWrite(obj.groupPath, node, "GroupPath");
    OptionalWrite(obj.groupSource, node, "GroupSource");
    return node;
}
template <>
bool Converter::decode(const Node& node, MdfChannel& obj)
{
    if (!node.IsMap())
    {
        throw ConversionError(node, "MdfChannel should be a Map");
    }
    OptionalRead(obj.channelName, node, "ChannelName");
    OptionalRead(obj.channelPath, node, "ChannelPath");
    OptionalRead(obj.channelSource, node, "ChannelSource");
    OptionalRead(obj.groupName, node, "GroupName");
    OptionalRead(obj.groupPath, node, "GroupPath");
    OptionalRead(obj.groupSource, node, "GroupSource");
    return true;
}

template <>
Node Converter::encode(const Replay& obj)
{
    static const Replay defaultObj{};
    Node node;
    node["UseTraceSource"] = obj.useTraceSource;
    NonDefaultWrite(obj.direction, node, "Direction", defaultObj.direction);
    NonDefaultWrite(obj.mdfChannel, node, "MdfChannel", defaultObj.mdfChannel);
    return node;
}
template <>
bool Converter::decode(const Node& node, Replay& obj)
{
    obj.useTraceSource = parse_as<decltype(obj.useTraceSource)>(node["UseTraceSource"]);
    OptionalRead(obj.direction, node, "Direction");
    OptionalRead(obj.mdfChannel, node, "MdfChannel");
    return true;
}

template <>
Node Converter::encode(const Replay::Direction& obj)
{
    Node node;
    switch (obj)
    {
    case Replay::Direction::Send:
        node = "Send";
        break;
    case Replay::Direction::Receive:
        node = "Receive";
        break;
    case Replay::Direction::Both:
        node = "Both";
        break;
    case Replay::Direction::Undefined:
        node = "Undefined";
        break;
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Replay::Direction& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Undefined" || str == "")
        obj = Replay::Direction::Undefined;
    else if (str == "Send")
        obj = Replay::Direction::Send;
    else if (str == "Receive")
        obj = Replay::Direction::Receive;
    else if (str == "Both")
        obj = Replay::Direction::Both;
    else
    {
        throw ConversionError(node, "Unknown Replay::Direction: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const CanController& obj)
{
    static const CanController defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, CanController& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.network, node, "Network");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const LinController& obj)
{
    static const LinController defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, LinController& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.network, node, "Network");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const EthernetController& obj)
{
    static const EthernetController defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.network, node, "Network");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");

    return node;
}
template <>
bool Converter::decode(const Node& node, EthernetController& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.network, node, "Network");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const Services::Flexray::FlexrayClusterParameters& obj)
{
    Node node;
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    node["gColdstartAttempts"] = static_cast<int>(obj.gColdstartAttempts);
    node["gCycleCountMax"] = static_cast<int>(obj.gCycleCountMax);
    node["gdActionPointOffset"] = static_cast<int>(obj.gdActionPointOffset);
    node["gdDynamicSlotIdlePhase"] = static_cast<int>(obj.gdDynamicSlotIdlePhase);
    node["gdMiniSlot"] = static_cast<int>(obj.gdMiniSlot);
    node["gdMiniSlotActionPointOffset"] = static_cast<int>(obj.gdMiniSlotActionPointOffset);
    node["gdStaticSlot"] = static_cast<int>(obj.gdStaticSlot);
    node["gdSymbolWindow"] = static_cast<int>(obj.gdSymbolWindow);
    node["gdSymbolWindowActionPointOffset"] = static_cast<int>(obj.gdSymbolWindowActionPointOffset);
    node["gdTSSTransmitter"] = static_cast<int>(obj.gdTSSTransmitter);
    node["gdWakeupTxActive"] = static_cast<int>(obj.gdWakeupTxActive);
    node["gdWakeupTxIdle"] = static_cast<int>(obj.gdWakeupTxIdle);
    node["gListenNoise"] = static_cast<int>(obj.gListenNoise);
    node["gMacroPerCycle"] = static_cast<int>(obj.gMacroPerCycle);
    node["gMaxWithoutClockCorrectionFatal"] = static_cast<int>(obj.gMaxWithoutClockCorrectionFatal);
    node["gMaxWithoutClockCorrectionPassive"] = static_cast<int>(obj.gMaxWithoutClockCorrectionPassive);
    node["gNumberOfMiniSlots"] = static_cast<int>(obj.gNumberOfMiniSlots);
    node["gNumberOfStaticSlots"] = static_cast<int>(obj.gNumberOfStaticSlots);
    node["gPayloadLengthStatic"] = static_cast<int>(obj.gPayloadLengthStatic);
    node["gSyncFrameIDCountMax"] = static_cast<int>(obj.gSyncFrameIDCountMax);
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayClusterParameters& obj)
{
    // Parse parameters as an int value; uint8_t would be interpreted as a character
    auto parseInt = [&node](auto instance, auto name) {
        return static_cast<decltype(instance)>(parse_as<int>(node[name]));
    };
    obj.gColdstartAttempts = parseInt(obj.gColdstartAttempts, "gColdstartAttempts");
    obj.gCycleCountMax = parseInt(obj.gCycleCountMax, "gCycleCountMax");
    obj.gdActionPointOffset = parseInt(obj.gdActionPointOffset, "gdActionPointOffset");
    obj.gdDynamicSlotIdlePhase = parseInt(obj.gdDynamicSlotIdlePhase, "gdDynamicSlotIdlePhase");
    obj.gdMiniSlot = parseInt(obj.gdMiniSlot, "gdMiniSlot");
    obj.gdMiniSlotActionPointOffset = parseInt(obj.gdMiniSlotActionPointOffset, "gdMiniSlotActionPointOffset");
    obj.gdStaticSlot = parseInt(obj.gdStaticSlot, "gdStaticSlot");
    obj.gdSymbolWindow = parseInt(obj.gdSymbolWindow, "gdSymbolWindow");
    obj.gdSymbolWindowActionPointOffset =
        parseInt(obj.gdSymbolWindowActionPointOffset, "gdSymbolWindowActionPointOffset");
    obj.gdTSSTransmitter = parseInt(obj.gdTSSTransmitter, "gdTSSTransmitter");
    obj.gdWakeupTxActive = parseInt(obj.gdWakeupTxActive, "gdWakeupTxActive");
    obj.gdWakeupTxIdle = parseInt(obj.gdWakeupTxIdle, "gdWakeupTxIdle");
    obj.gListenNoise = parseInt(obj.gListenNoise, "gListenNoise");
    obj.gMacroPerCycle = parseInt(obj.gMacroPerCycle, "gMacroPerCycle");
    obj.gMaxWithoutClockCorrectionFatal =
        parseInt(obj.gMaxWithoutClockCorrectionFatal, "gMaxWithoutClockCorrectionFatal");
    obj.gMaxWithoutClockCorrectionPassive =
        parseInt(obj.gMaxWithoutClockCorrectionPassive, "gMaxWithoutClockCorrectionPassive");
    obj.gNumberOfMiniSlots = parseInt(obj.gNumberOfMiniSlots, "gNumberOfMiniSlots");
    obj.gNumberOfStaticSlots = parseInt(obj.gNumberOfStaticSlots, "gNumberOfStaticSlots");
    obj.gPayloadLengthStatic = parseInt(obj.gPayloadLengthStatic, "gPayloadLengthStatic");
    obj.gSyncFrameIDCountMax = parseInt(obj.gSyncFrameIDCountMax, "gSyncFrameIDCountMax");
    return true;
}
template <>
Node Converter::encode(const Services::Flexray::FlexrayNodeParameters& obj)
{
    Node node;
    node["pAllowHaltDueToClock"] = static_cast<int>(obj.pAllowHaltDueToClock);
    node["pAllowPassiveToActive"] = static_cast<int>(obj.pAllowPassiveToActive);
    node["pClusterDriftDamping"] = static_cast<int>(obj.pClusterDriftDamping);
    node["pdAcceptedStartupRange"] = static_cast<int>(obj.pdAcceptedStartupRange);
    node["pdListenTimeout"] = static_cast<int>(obj.pdListenTimeout);
    node["pKeySlotId"] = static_cast<int>(obj.pKeySlotId);
    node["pKeySlotOnlyEnabled"] = static_cast<int>(obj.pKeySlotOnlyEnabled);
    node["pKeySlotUsedForStartup"] = static_cast<int>(obj.pKeySlotUsedForStartup);
    node["pKeySlotUsedForSync"] = static_cast<int>(obj.pKeySlotUsedForSync);
    node["pLatestTx"] = static_cast<int>(obj.pLatestTx);
    node["pMacroInitialOffsetA"] = static_cast<int>(obj.pMacroInitialOffsetA);
    node["pMacroInitialOffsetB"] = static_cast<int>(obj.pMacroInitialOffsetB);
    node["pMicroInitialOffsetA"] = static_cast<int>(obj.pMicroInitialOffsetA);
    node["pMicroInitialOffsetB"] = static_cast<int>(obj.pMicroInitialOffsetB);
    node["pMicroPerCycle"] = static_cast<int>(obj.pMicroPerCycle);
    node["pOffsetCorrectionOut"] = static_cast<int>(obj.pOffsetCorrectionOut);
    node["pOffsetCorrectionStart"] = static_cast<int>(obj.pOffsetCorrectionStart);
    node["pRateCorrectionOut"] = static_cast<int>(obj.pRateCorrectionOut);
    node["pWakeupPattern"] = static_cast<int>(obj.pWakeupPattern);
    node["pSamplesPerMicrotick"] = static_cast<int>(obj.pSamplesPerMicrotick);
    node["pWakeupChannel"] = obj.pWakeupChannel;
    node["pdMicrotick"] = obj.pdMicrotick;
    node["pChannels"] = obj.pChannels;
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayNodeParameters& obj)
{
    auto parseInt = [&node](auto instance, auto name) {
        return static_cast<decltype(instance)>(parse_as<int>(node[name]));
    };
    obj.pAllowHaltDueToClock = parseInt(obj.pAllowHaltDueToClock, "pAllowHaltDueToClock");
    obj.pAllowPassiveToActive = parseInt(obj.pAllowPassiveToActive, "pAllowPassiveToActive");
    obj.pClusterDriftDamping = parseInt(obj.pClusterDriftDamping, "pClusterDriftDamping");
    obj.pdAcceptedStartupRange = parseInt(obj.pdAcceptedStartupRange, "pdAcceptedStartupRange");
    obj.pdListenTimeout = parseInt(obj.pdListenTimeout, "pdListenTimeout");
    obj.pKeySlotId = parseInt(obj.pKeySlotId, "pKeySlotId");
    obj.pKeySlotOnlyEnabled = parseInt(obj.pKeySlotOnlyEnabled, "pKeySlotOnlyEnabled");
    obj.pKeySlotUsedForStartup = parseInt(obj.pKeySlotUsedForStartup, "pKeySlotUsedForStartup");
    obj.pKeySlotUsedForSync = parseInt(obj.pKeySlotUsedForSync, "pKeySlotUsedForSync");
    obj.pLatestTx = parseInt(obj.pLatestTx, "pLatestTx");
    obj.pMacroInitialOffsetA = parseInt(obj.pMacroInitialOffsetA, "pMacroInitialOffsetA");
    obj.pMacroInitialOffsetB = parseInt(obj.pMacroInitialOffsetB, "pMacroInitialOffsetB");
    obj.pMicroInitialOffsetA = parseInt(obj.pMicroInitialOffsetA, "pMicroInitialOffsetA");
    obj.pMicroInitialOffsetB = parseInt(obj.pMicroInitialOffsetB, "pMicroInitialOffsetB");
    obj.pMicroPerCycle = parseInt(obj.pMicroPerCycle, "pMicroPerCycle");
    obj.pOffsetCorrectionOut = parseInt(obj.pOffsetCorrectionOut, "pOffsetCorrectionOut");
    obj.pOffsetCorrectionStart = parseInt(obj.pOffsetCorrectionStart, "pOffsetCorrectionStart");
    obj.pRateCorrectionOut = parseInt(obj.pRateCorrectionOut, "pRateCorrectionOut");
    obj.pWakeupPattern = parseInt(obj.pWakeupPattern, "pWakeupPattern");
    obj.pSamplesPerMicrotick = parseInt(obj.pSamplesPerMicrotick, "pSamplesPerMicrotick");
    obj.pWakeupChannel = parse_as<Services::Flexray::FlexrayChannel>(node["pWakeupChannel"]);
    obj.pdMicrotick = parse_as<Services::Flexray::FlexrayClockPeriod>(node["pdMicrotick"]);
    obj.pChannels = parse_as<Services::Flexray::FlexrayChannel>(node["pChannels"]);
    return true;
}

template <>
Node Converter::encode(const Services::Flexray::FlexrayTxBufferConfig& obj)
{
    Node node;
    node["channels"] = obj.channels;
    node["slotId"] = obj.slotId;
    node["offset"] = static_cast<int>(obj.offset);
    node["repetition"] = static_cast<int>(obj.repetition);
    node["PPindicator"] = obj.hasPayloadPreambleIndicator;
    node["headerCrc"] = obj.headerCrc;
    node["transmissionMode"] = obj.transmissionMode;
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayTxBufferConfig& obj)
{
    obj.channels = parse_as<Services::Flexray::FlexrayChannel>(node["channels"]);
    obj.slotId = parse_as<uint16_t>(node["slotId"]);
    obj.offset = static_cast<uint8_t>(parse_as<int>(node["offset"]));
    obj.repetition = static_cast<uint8_t>(parse_as<int>(node["repetition"]));
    obj.hasPayloadPreambleIndicator = parse_as<bool>(node["PPindicator"]);
    obj.headerCrc = parse_as<uint16_t>(node["headerCrc"]);
    obj.transmissionMode = parse_as<Services::Flexray::FlexrayTransmissionMode>(node["transmissionMode"]);
    return true;
}

template <>
Node Converter::encode(const Services::Flexray::FlexrayChannel& obj)
{
    Node node;
    switch (obj)
    {
    case Services::Flexray::FlexrayChannel::A:
        node = "A";
        break;
    case Services::Flexray::FlexrayChannel::B:
        node = "B";
        break;
    case Services::Flexray::FlexrayChannel::AB:
        node = "AB";
        break;
    case Services::Flexray::FlexrayChannel::None:
        node = "None";
        break;
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayChannel& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "A")
        obj = Services::Flexray::FlexrayChannel::A;
    else if (str == "B")
        obj = Services::Flexray::FlexrayChannel::B;
    else if (str == "AB")
        obj = Services::Flexray::FlexrayChannel::AB;
    else if (str == "None" || str == "")
        obj = Services::Flexray::FlexrayChannel::None;
    else
    {
        throw ConversionError(node, "Unknown Services::Flexray::FlexrayChannel: " + str);
    }
    return true;
}

template <>
Node Converter::encode(const Services::Flexray::FlexrayClockPeriod& obj)
{
    Node node;
    switch (obj)
    {
    case Services::Flexray::FlexrayClockPeriod::T12_5NS:
        node = "12.5ns";
        break;
    case Services::Flexray::FlexrayClockPeriod::T25NS:
        node = "25ns";
        break;
    case Services::Flexray::FlexrayClockPeriod::T50NS:
        node = "50ns";
        break;
    default:
        throw ConversionError(node, "Unknown Services::Flexray::FlexrayClockPeriod");
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayClockPeriod& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "12.5ns")
        obj = Services::Flexray::FlexrayClockPeriod::T12_5NS;
    else if (str == "25ns")
        obj = Services::Flexray::FlexrayClockPeriod::T25NS;
    else if (str == "50ns")
        obj = Services::Flexray::FlexrayClockPeriod::T50NS;
    else
    {
        throw ConversionError(node, "Unknown Services::Flexray::FlexrayClockPeriod: " + str);
    }
    return true;
}

template <>
Node Converter::encode(const Services::Flexray::FlexrayTransmissionMode& obj)
{
    Node node;
    switch (obj)
    {
    case Services::Flexray::FlexrayTransmissionMode::Continuous:
        node = "Continuous";
        break;
    case Services::Flexray::FlexrayTransmissionMode::SingleShot:
        node = "SingleShot";
        break;
    default:
        throw ConversionError(node, "Unknown FlexrayTransmissionMode");
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Services::Flexray::FlexrayTransmissionMode& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Continuous")
        obj = Services::Flexray::FlexrayTransmissionMode::Continuous;
    else if (str == "SingleShot")
        obj = Services::Flexray::FlexrayTransmissionMode::SingleShot;
    else
    {
        throw ConversionError(node, "Unknown Services::Flexray::FlexrayTransmissionMode: " + str);
    }
    return true;
}

template <>
Node Converter::encode(const FlexrayController& obj)
{
    static const FlexrayController defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.network, node, "Network");
    if (obj.clusterParameters.has_value())
    {
        node["ClusterParameters"] = encode(obj.clusterParameters.value());
    }
    if (obj.nodeParameters.has_value())
    {
        node["NodeParameters"] = encode(obj.nodeParameters.value());
    }
    OptionalWrite(obj.clusterParameters, node, "ClusterParameters");
    OptionalWrite(obj.nodeParameters, node, "NodeParameters");
    OptionalWrite(obj.txBufferConfigurations, node, "TxBufferConfigurations");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, FlexrayController& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.network, node, "Network");
    OptionalRead(obj.clusterParameters, node, "ClusterParameters");
    OptionalRead(obj.nodeParameters, node, "NodeParameters");
    OptionalRead(obj.txBufferConfigurations, node, "TxBufferConfigurations");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const Label::Kind& obj)
{
    Node node;
    node = static_cast<std::underlying_type_t<Label::Kind>>(obj);
    switch (obj)
    {
    case Label::Kind::Mandatory:
        node = "Mandatory";
        break;
    case Label::Kind::Optional:
        node = "Optional";
        break;
    default:
        throw ConfigurationError{"Unknown MatchingLabel::Kind"};
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Label::Kind& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Mandatory")
        obj = Label::Kind::Mandatory;
    else if (str == "Optional")
        obj = Label::Kind::Optional;
    else
    {
        throw ConversionError(node, "Unknown MatchingLabel::Kind: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const Label& obj)
{
    Node node;
    node["Key"] = obj.key;
    node["Value"] = obj.value;
    node["Kind"] = encode(obj.kind);
    return node;
}
template <>
bool Converter::decode(const Node& node, Label& obj)
{
    OptionalRead(obj.key, node, "Key");
    OptionalRead(obj.value, node, "Value");
    OptionalRead(obj.kind, node, "Kind");
    return true;
}

template <>
Node Converter::encode(const DataPublisher& obj)
{
    static const DataPublisher defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.topic, node, "Topic");
    OptionalWrite(obj.labels, node, "Labels");
    //OptionalWrite(obj.history, node, "History");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, DataPublisher& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.topic, node, "Topic");
    OptionalRead(obj.labels, node, "Labels");
    //OptionalRead(obj.history, node, "Replay");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const DataSubscriber& obj)
{
    static const DataSubscriber defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.topic, node, "Topic");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, DataSubscriber& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead(obj.topic, node, "Topic");
    OptionalRead(obj.labels, node, "Labels");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const RpcServer& obj)
{
    static const RpcServer defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.functionName, node, "FunctionName");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, RpcServer& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead_deprecated_alternative(obj.functionName, node, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj.labels, node, "Labels");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const RpcClient& obj)
{
    static const RpcClient defaultObj{};
    Node node;
    node["Name"] = obj.name;
    OptionalWrite(obj.functionName, node, "Channel");
    OptionalWrite(obj.labels, node, "Labels");
    OptionalWrite(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalWrite(obj.replay, node, "Replay");
    return node;
}
template <>
bool Converter::decode(const Node& node, RpcClient& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    OptionalRead_deprecated_alternative(obj.functionName, node, "FunctionName", {"Channel", "RpcChannel"});
    OptionalRead(obj.labels, node, "Labels");
    OptionalRead(obj.useTraceSinks, node, "UseTraceSinks");
    OptionalRead(obj.replay, node, "Replay");
    return true;
}

template <>
Node Converter::encode(const HealthCheck& obj)
{
    Node node;
    OptionalWrite(obj.softResponseTimeout, node, "SoftResponseTimeout");
    OptionalWrite(obj.hardResponseTimeout, node, "HardResponseTimeout");
    return node;
}
template <>
bool Converter::decode(const Node& node, HealthCheck& obj)
{
    OptionalRead(obj.softResponseTimeout, node, "SoftResponseTimeout");
    OptionalRead(obj.hardResponseTimeout, node, "HardResponseTimeout");
    return true;
}

template <>
Node Converter::encode(const Tracing& obj)
{
    static const Tracing defaultObj{};
    Node node;
    OptionalWrite(obj.traceSinks, node, "TraceSinks");
    OptionalWrite(obj.traceSources, node, "TraceSources");
    return node;
}
template <>
bool Converter::decode(const Node& node, Tracing& obj)
{
    OptionalRead(obj.traceSinks, node, "TraceSinks");
    OptionalRead(obj.traceSources, node, "TraceSources");
    return true;
}

template <>
Node Converter::encode(const TraceSink& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["Type"] = obj.type;
    node["OutputPath"] = obj.outputPath;
    // Only serialize if disabled
    //if (!obj.enabled)
    //{
    //    node["Enabled"] = obj.enabled;
    //}

    return node;
}
template <>
bool Converter::decode(const Node& node, TraceSink& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.type = parse_as<decltype(obj.type)>(node["Type"]);
    obj.outputPath = parse_as<decltype(obj.outputPath)>(node["OutputPath"]);
    //if (node["Enabled"])
    //{
    //    obj.enabled = parse_as<decltype(obj.enabled)>(node["Enabled"]);
    //}
    return true;
}

template <>
Node Converter::encode(const TraceSink::Type& obj)
{
    Node node;
    switch (obj)
    {
    case TraceSink::Type::Undefined:
        node = "Undefined";
        break;
    case TraceSink::Type::Mdf4File:
        node = "Mdf4File";
        break;
    case TraceSink::Type::PcapFile:
        node = "PcapFile";
        break;
    case TraceSink::Type::PcapPipe:
        node = "PcapPipe";
        break;
    default:
        throw ConfigurationError{"Unknown TraceSink Type"};
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, TraceSink::Type& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Undefined" || str == "")
        obj = TraceSink::Type::Undefined;
    else if (str == "Mdf4File")
        obj = TraceSink::Type::Mdf4File;
    else if (str == "PcapFile")
        obj = TraceSink::Type::PcapFile;
    else if (str == "PcapPipe")
        obj = TraceSink::Type::PcapPipe;
    else
    {
        throw ConversionError(node, "Unknown TraceSink::Type: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const TraceSource& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["Type"] = obj.type;
    node["InputPath"] = obj.inputPath;
    // Only serialize if disabled
    //if (!obj.enabled)
    //{
    //    node["Enabled"] = obj.enabled;
    //}
    return node;
}
template <>
bool Converter::decode(const Node& node, TraceSource& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.type = parse_as<decltype(obj.type)>(node["Type"]);
    obj.inputPath = parse_as<decltype(obj.inputPath)>(node["InputPath"]);
    //if (node["Enabled"])
    //{
    //    obj.enabled = parse_as<decltype(obj.enabled)>(node["Enabled"]);
    //}
    return true;
}

template <>
Node Converter::encode(const TraceSource::Type& obj)
{
    Node node;
    switch (obj)
    {
    case TraceSource::Type::Undefined:
        node = "Undefined";
        break;
    case TraceSource::Type::Mdf4File:
        node = "Mdf4File";
        break;
    case TraceSource::Type::PcapFile:
        node = "PcapFile";
        break;
    default:
        throw ConfigurationError{"Unknown TraceSource Type"};
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, TraceSource::Type& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Undefined" || str == "")
        obj = TraceSource::Type::Undefined;
    else if (str == "Mdf4File")
        obj = TraceSource::Type::Mdf4File;
    else if (str == "PcapFile")
        obj = TraceSource::Type::PcapFile;
    else
    {
        throw ConversionError(node, "Unknown TraceSource::Type: " + str + ".");
    }
    return true;
}

// Metrics

template <>
Node Converter::encode(const MetricsSink::Type& obj)
{
    Node node;
    switch (obj)
    {
    case MetricsSink::Type::Undefined:
        node = "Undefined";
        break;
    case MetricsSink::Type::JsonFile:
        node = "JsonFile";
        break;
    case MetricsSink::Type::Remote:
        node = "Remote";
        break;
    default:
        throw ConfigurationError{"Unknown MetricsSink Type"};
    }
    return node;
}

template <>
bool Converter::decode(const Node& node, MetricsSink::Type& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Undefined" || str == "")
    {
        obj = MetricsSink::Type::Undefined;
    }
    else if (str == "JsonFile")
    {
        obj = MetricsSink::Type::JsonFile;
    }
    else if (str == "Remote")
    {
        obj = MetricsSink::Type::Remote;
    }
    else
    {
        throw ConversionError{node, "Unknown MetricsSink::Type: " + str + "."};
    }
    return true;
}

template <>
Node Converter::encode(const MetricsSink& obj)
{
    Node node;
    node["Type"] = obj.type;
    if (!obj.name.empty())
    {
        node["Name"] = obj.name;
    }
    return node;
}

template <>
bool Converter::decode(const Node& node, MetricsSink& obj)
{
    obj.type = parse_as<decltype(obj.type)>(node["Type"]);
    OptionalRead(obj.name, node, "Name");
    return true;
}

template <>
Node Converter::encode(const Metrics& obj)
{
    Node node;
    OptionalWrite(obj.sinks, node, "Sinks");
    if (obj.collectFromRemote)
    {
        node["CollectFromRemote"] = obj.collectFromRemote;
    }
    return node;
}

template <>
bool Converter::decode(const Node& node, Metrics& obj)
{
    OptionalRead(obj.sinks, node, "Sinks");
    OptionalRead(obj.collectFromRemote, node, "CollectFromRemote");
    return true;
}

// Extensions

template <>
Node Converter::encode(const Extensions& obj)
{
    static const Extensions defaultObj{};
    Node node;
    NonDefaultWrite(obj.searchPathHints, node, "SearchPathHints", defaultObj.searchPathHints);
    return node;
}
template <>
bool Converter::decode(const Node& node, Extensions& obj)
{
    OptionalRead(obj.searchPathHints, node, "SearchPathHints");
    return true;
}


template <>
Node Converter::encode(const Middleware& obj)
{
    Node node;
    static const Middleware defaultObj;
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
    return node;
}
template <>
bool Converter::decode(const Node& node, Middleware& obj)
{
    OptionalRead(obj.registryUri, node, "RegistryUri");
    OptionalRead(obj.connectAttempts, node, "ConnectAttempts");
    OptionalRead(obj.tcpNoDelay, node, "TcpNoDelay");
    OptionalRead(obj.tcpQuickAck, node, "TcpQuickAck");
    OptionalRead(obj.tcpReceiveBufferSize, node, "TcpReceiveBufferSize");
    OptionalRead(obj.tcpSendBufferSize, node, "TcpSendBufferSize");
    OptionalRead(obj.enableDomainSockets, node, "EnableDomainSockets");
    OptionalRead(obj.acceptorUris, node, "AcceptorUris");
    OptionalRead(obj.registryAsFallbackProxy, node, "RegistryAsFallbackProxy");
    OptionalRead(obj.experimentalRemoteParticipantConnection, node, "ExperimentalRemoteParticipantConnection");
    OptionalRead(obj.connectTimeoutSeconds, node, "ConnectTimeoutSeconds");
    return true;
}

template <>
Node Converter::encode(const Aggregation& obj)
{
    Node node;
    switch (obj)
    {
    case Aggregation::Off:
        node = "Off";
        break;
    case Aggregation::On:
        node = "On";
        break;
    case Aggregation::Auto:
        node = "Auto";
        break;
    default:
        throw ConfigurationError{"Unknown Aggregation Type"};
    }
    return node;
}
template <>
bool Converter::decode(const Node& node, Aggregation& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "Off" || str == "")
        obj = Aggregation::Off;
    else if (str == "On")
        obj = Aggregation::On;
    else if (str == "Auto")
        obj = Aggregation::Auto;
    else
    {
        throw ConversionError(node, "Unknown Aggregation: " + str + ".");
    }
    return true;
}

template <>
Node Converter::encode(const TimeSynchronization& obj)
{
    Node node;
    static const TimeSynchronization defaultObj;
    NonDefaultWrite(obj.animationFactor, node, "AnimationFactor", defaultObj.animationFactor);
    NonDefaultWrite(obj.enableMessageAggregation, node, "EnableMessageAggregation",
                       defaultObj.enableMessageAggregation);
    return node;
}
template <>
bool Converter::decode(const Node& node, TimeSynchronization& obj)
{
    OptionalRead(obj.animationFactor, node, "AnimationFactor");
    OptionalRead(obj.enableMessageAggregation, node, "EnableMessageAggregation");
    return true;
}

template <>
Node Converter::encode(const Experimental& obj)
{
    static const Experimental defaultObj{};

    Node node;
    NonDefaultWrite(obj.timeSynchronization, node, "TimeSynchronization", defaultObj.timeSynchronization);
    NonDefaultWrite(obj.metrics, node, "Metrics", defaultObj.metrics);
    return node;
}
template <>
bool Converter::decode(const Node& node, Experimental& obj)
{
    OptionalRead(obj.timeSynchronization, node, "TimeSynchronization");
    OptionalRead(obj.metrics, node, "Metrics");
    return true;
}


template <>
Node Converter::encode(const ParticipantConfiguration& obj)
{
    static const ParticipantConfiguration defaultObj{};
    Node node;
    node["SchemaVersion"] = obj.schemaVersion;
    if (!obj.description.empty())
    {
        node["Description"] = obj.description;
    }
    if (!obj.participantName.empty())
    {
        node["ParticipantName"] = obj.participantName;
    }

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
    NonDefaultWrite(obj.experimental, node, "Experimental", defaultObj.experimental);
    return node;
}
template <>
bool Converter::decode(const Node& node, ParticipantConfiguration& obj)
{
    OptionalRead(obj.schemaVersion, node, "SchemaVersion");
    OptionalRead(obj.description, node, "Description");
    OptionalRead(obj.participantName, node, "ParticipantName");

    OptionalRead(obj.canControllers, node, "CanControllers");
    OptionalRead(obj.linControllers, node, "LinControllers");
    OptionalRead(obj.ethernetControllers, node, "EthernetControllers");
    OptionalRead_deprecated_alternative(obj.flexrayControllers, node, "FlexrayControllers", {"FlexRayControllers"});
    OptionalRead(obj.dataPublishers, node, "DataPublishers");
    OptionalRead(obj.dataSubscribers, node, "DataSubscribers");
    OptionalRead(obj.rpcServers, node, "RpcServers");
    OptionalRead(obj.rpcClients, node, "RpcClients");

    OptionalRead(obj.logging, node, "Logging");
    OptionalRead(obj.healthCheck, node, "HealthCheck");
    OptionalRead(obj.tracing, node, "Tracing");
    OptionalRead(obj.extensions, node, "Extensions");
    OptionalRead(obj.middleware, node, "Middleware");
    OptionalRead(obj.experimental, node, "Experimental");
    return true;
}

// Conversions for ServiceDiscovery Supplemental Data

template <>
Node Converter::encode(const SilKit::Services::MatchingLabel::Kind& obj)
{
    Node node;
    node = static_cast<std::underlying_type_t<SilKit::Services::MatchingLabel::Kind>>(obj);
    return node;
}
template <>
bool Converter::decode(const Node& node, SilKit::Services::MatchingLabel::Kind& obj)
{
    obj = static_cast<SilKit::Services::MatchingLabel::Kind>(
        parse_as<std::underlying_type_t<SilKit::Services::MatchingLabel::Kind>>(node));
    return true;
}

template <>
Node Converter::encode(const SilKit::Services::MatchingLabel& obj)
{
    Node node;
    node["key"] = obj.key;
    node["value"] = obj.value;
    node["kind"] = encode(obj.kind);
    return node;
}
template <>
bool Converter::decode(const Node& node, SilKit::Services::MatchingLabel& obj)
{
    OptionalRead(obj.key, node, "key");
    OptionalRead(obj.value, node, "value");
    OptionalRead(obj.kind, node, "kind");
    return true;
}

} // namespace YAML
