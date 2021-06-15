// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"

#include <algorithm>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "ib/cfg/OptionalCfg.hpp"

//local utilities
namespace {
    using namespace ib::cfg;

    auto macaddress_encode(const std::array<uint8_t, 6>& macAddress) -> YAML::Node
    {
        std::stringstream macOut;

        to_ostream(macOut, macAddress);

        return YAML::Node(macOut.str());
    }

    auto macaddress_decode(const YAML::Node& node) -> std::array<uint8_t, 6>
    {
        std::array<uint8_t, 6> macAddress;

        std::stringstream macIn(node.as<std::string>());
        from_istream(macIn, macAddress);

        return macAddress;
    }

    template<typename ConfigT>
    void optional_encode(const OptionalCfg<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
    {
        if (value)
        {
            node[fieldName] = value.value();
        }
    }

    template<typename ConfigT>
    void optional_decode(OptionalCfg<ConfigT>& value, const YAML::Node& node, const std::string& fieldName)
    {
        if (node[fieldName]) //operator[] does not modify node
        {
            value = node[fieldName].as<ConfigT>();
        }
    }

    template<typename ConfigT>
    void optional_decode(ConfigT& value, const YAML::Node& node, const std::string& fieldName)
    {
        if (node[fieldName]) //operator[] does not modify node
        {
            value = node[fieldName].as<ConfigT>();
        }
    }

    template <typename ConfigT>
    auto non_default_encode(const std::vector<ConfigT>& values, YAML::Node& node, const std::string& fieldName, const ConfigT& defaultValue)
    {
        if (!(value == defaultValue))
        {
            auto&& sequence = node[fieldName];
            std::copy(values.begin(), values.end(), sequence.begin());
        }
    }

    template <typename ConfigT>
    auto non_default_encode(const ConfigT& value, YAML::Node& node, const std::string& fieldName, const ConfigT& defaultValue)
    {
        if (!(value == defaultValue))
            node[fieldName] = value;
    }

} //end anonymous

// YAML type conversion helpers for our data types
namespace YAML {

using namespace ib;

template<>
Node VibConversion::encode(const MdfChannel& obj)
{
    Node node;
    optional_encode(obj.channelName, node, "ChannelName");
    optional_encode(obj.channelPath, node, "ChannelPath");
    optional_encode(obj.channelSource, node, "ChannelSource");
    optional_encode(obj.groupName, node, "GroupName");
    optional_encode(obj.groupPath, node, "GroupPath");
    optional_encode(obj.groupSource, node, "GroupSource");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, MdfChannel& obj)
{
    if (!node.IsMap())
    {
        return false;
    }
    optional_decode(obj.channelName, node, "ChannelName");
    optional_decode(obj.channelPath, node, "ChannelPath");
    optional_decode(obj.channelSource, node, "ChannelSource");
    optional_decode(obj.groupName, node, "GroupName");
    optional_decode(obj.groupPath, node, "GroupPath");
    optional_decode(obj.groupSource, node, "GroupSource");
    return true;
}

// copy paste
template<>
Node VibConversion::encode(const Version& obj)
{
    Node node;
    std::stringstream ss;
    ss << obj;
    node = ss.str();
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Version& obj)
{
    if (!node.IsScalar())
    {
        return false;
    }
    std::stringstream in(node.as<std::string>());
    in >> obj;
    return !in.fail();
}

template<>
Node VibConversion::encode(const Sink::Type& obj)
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
    default:
        break; 
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Sink::Type& obj)
{
    if (!node.IsScalar())
    {
        return false;
    }
    auto&& str = node.as<std::string>();
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
        return false;
    }
    return true;
}

template<>
Node VibConversion::encode(const mw::logging::Level& obj)
{
    Node node;
    switch (obj)
    {
    case mw::logging::Level::Critical:
        node = "Critical";
        break;
    case mw::logging::Level::Error:
        node = "Error";
        break;
    case mw::logging::Level::Warn:
        node = "Warn";
        break;
    case mw::logging::Level::Info:
        node = "Info";
        break;
    case mw::logging::Level::Debug:
        node = "Debug";
        break;
    case mw::logging::Level::Trace:
        node = "Trace";
        break;
    case mw::logging::Level::Off:
        node =  "Off";
        break;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, mw::logging::Level& obj)
{
    auto&& str = node.as<std::string>();
    if (str == "Critical")
        obj = mw::logging::Level::Critical;
    else if (str == "Error")
        obj = mw::logging::Level::Error;
    else if (str == "Warn")
        obj = mw::logging::Level::Warn;
    else if (str == "Info")
        obj = mw::logging::Level::Info;
    else if (str == "Debug")
        obj = mw::logging::Level::Debug;
    else if (str == "Trace")
        obj = mw::logging::Level::Trace;
    else if (str == "Off")
        obj = mw::logging::Level::Off;
    else
        return false;
    return true;
}

template<>
Node VibConversion::encode(const Sink& obj)
{
    static const Sink defaultSink;
    Node node;
    non_default_encode(obj.type, node, "Type", defaultSink.type);
    non_default_encode(obj.level, node, "Level", defaultSink.level);
    non_default_encode(obj.logname, node, "Logname", defaultSink.logname);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Sink& obj)
{
    obj.type = node["Type"].as<Sink::Type>(); 
    optional_decode(obj.level, node, "Level");

    if (obj.type == Sink::Type::File)
    {
        if (!node["Logname"])
        {
            return false; //XXX throw here with message?
        }
        obj.logname = node["Logname"].as<std::string>();
    }

    return true;
}

template<>
Node VibConversion::encode(const Logger& obj)
{
    Node node;
    static const Logger defaultLogger;

    non_default_encode(obj.logFromRemotes, node, "LogFromRemotes", defaultLogger.logFromRemotes);
    non_default_encode(obj.flush_level, node, "FlushLevel", defaultLogger.flush_level);
    non_default_encode(obj.sinks, node, "Sinks", defaultLogger.sinks);

    return node;
}
template<>
bool VibConversion::decode(const Node& node, Logger& obj)
{
    optional_decode(obj.logFromRemotes, node, "LogFromRemotes");
    optional_decode(obj.flush_level, node, "FlushLevel");
    optional_decode(obj.sinks, node, "Sinks");
    return true;
}

template<>
Node VibConversion::encode(const CanController& obj)
{
    static const CanController defaultObj;
    Node node;
    node["Name"] = obj.name;
    non_default_encode(obj.useTraceSinks, node, "UseTraceSinks", defaultObj.useTraceSinks);
    if (!obj.useTraceSinks.empty())
    {
        node["Replay"] = obj.replay;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, CanController& obj)
{
    // backward compatibility to old json config files with only name of controller
    if (node.IsScalar())
    {
        obj.name = node.as<std::string>();
        return true;
    }

    obj.name = node["Name"].as<std::string>();
    if (node["UseTraceSinks"])
    {
        obj.useTraceSinks = node["UseTraceSinks"].as<decltype(obj.useTraceSinks)>();
    }
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const LinController& obj)
{
    Node node;
    static const LinController defaultObj;
    node["Name"] = obj.name;

    non_default_encode(obj.useTraceSinks, node, "UseTraceSinks", defaultObj.useTraceSinks);

    if (!obj.useTraceSinks.empty())
    {
        node["Replay"] = obj.replay;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, LinController& obj)
{
    // backward compatibility to old json config files with only name of controller
    if (node.IsScalar())
    {
        obj.name = node.as<std::string>();
        return true;
    }

    obj.name = node["Name"].as<std::string>();
    if (node["UseTraceSinks"])
    {
        obj.useTraceSinks = node["UseTraceSinks"].as<decltype(obj.useTraceSinks)>();
    }

    optional_decode(obj.replay, node, "Replay");
    return false;
}

template<>
Node VibConversion::encode(const EthernetController& obj)
{
    Node node;
    static const EthernetController defaultObj;
    node["Name"] = obj.name;
    node["MacAddr"] = macaddress_encode(obj.macAddress);
    if (!obj.pcapFile.empty())
    {
        node["PcapFile"] = obj.pcapFile;
    }

    if (!obj.pcapPipe.empty())
    {
        node["PcapPipe"] = obj.pcapPipe;
    }

    non_default_encode(obj.useTraceSinks, node, "UseTraceSinks", defaultObj.useTraceSinks);
    if (!obj.useTraceSinks.empty())
    {
        node["Replay"] = obj.replay;
    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, EthernetController& obj)
{
    obj.name = node["Name"].as<std::string>();
    obj.macAddress = macaddress_decode(node["MacAddr"]);
    optional_decode(obj.pcapFile, node, "PcapFile");
    optional_decode(obj.pcapPipe, node, "PcapPipe");
    if (node["UseTraceSinks"])
    {
        obj.useTraceSinks = node["UseTraceSinks"].as<decltype(obj.useTraceSinks)>();
    }
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::ClusterParameters& obj)
{
    Node node;

    node["gColdstartAttempts"] = obj.gColdstartAttempts;
    node["gCycleCountMax"] = obj.gCycleCountMax;
    node["gdActionPointOffset"] = obj.gdActionPointOffset;
    node["gdDynamicSlotIdlePhase"] = obj.gdDynamicSlotIdlePhase;
    node["gdMiniSlot"] = obj.gdMiniSlot;
    node["gdMiniSlotActionPointOffset"] = obj.gdMiniSlotActionPointOffset;
    node["gdStaticSlot"] = obj.gdStaticSlot;
    node["gdSymbolWindow"] = obj.gdSymbolWindow;
    node["gdSymbolWindowActionPointOffset"] = obj.gdSymbolWindowActionPointOffset;
    node["gdTSSTransmitter"] = obj.gdTSSTransmitter;
    node["gdWakeupTxActive"] = obj.gdWakeupTxActive;
    node["gdWakeupTxIdle"] = obj.gdWakeupTxIdle;
    node["gListenNoise"] = obj.gListenNoise;
    node["gMacroPerCycle"] = obj.gMacroPerCycle;
    node["gMaxWithoutClockCorrectionFatal"] = obj.gMaxWithoutClockCorrectionFatal;
    node["gMaxWithoutClockCorrectionPassive"] = obj.gMaxWithoutClockCorrectionPassive;
    node["gNumberOfMiniSlots"] = obj.gNumberOfMiniSlots;
    node["gNumberOfStaticSlots"] = obj.gNumberOfStaticSlots;
    node["gPayloadLengthStatic"] = obj.gPayloadLengthStatic;
    node["gSyncFrameIDCountMax"] = obj.gSyncFrameIDCountMax;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::ClusterParameters& obj)
{
    obj.gColdstartAttempts = node["gColdstartAttempts"].as<int>();
    obj.gCycleCountMax = node["gCycleCountMax"].as<int>();
    obj.gdActionPointOffset = node["gdActionPointOffset"].as<int>();
    obj.gdDynamicSlotIdlePhase = node["gdDynamicSlotIdlePhase"].as<int>();
    obj.gdMiniSlot = node["gdMiniSlot"].as<int>();
    obj.gdMiniSlotActionPointOffset = node["gdMiniSlotActionPointOffset"].as<int>();
    obj.gdStaticSlot = node["gdStaticSlot"].as<int>();
    obj.gdSymbolWindow = node["gdSymbolWindow"].as<int>();
    obj.gdSymbolWindowActionPointOffset = node["gdSymbolWindowActionPointOffset"].as<int>();
    obj.gdTSSTransmitter = node["gdTSSTransmitter"].as<int>();
    obj.gdWakeupTxActive = node["gdWakeupTxActive"].as<int>();
    obj.gdWakeupTxIdle = node["gdWakeupTxIdle"].as<int>();
    obj.gListenNoise = node["gListenNoise"].as<int>();
    obj.gMacroPerCycle = node["gMacroPerCycle"].as<int>();
    obj.gMaxWithoutClockCorrectionFatal = node["gMaxWithoutClockCorrectionFatal"].as<int>();
    obj.gMaxWithoutClockCorrectionPassive = node["gMaxWithoutClockCorrectionPassive"].as<int>();
    obj.gNumberOfMiniSlots = node["gNumberOfMiniSlots"].as<int>();
    obj.gNumberOfStaticSlots = node["gNumberOfStaticSlots"].as<int>();
    obj.gPayloadLengthStatic = node["gPayloadLengthStatic"].as<int>();
    obj.gSyncFrameIDCountMax = node["gSyncFrameIDCountMax"].as<int>();
    return false;
}
template<>
Node VibConversion::encode(const sim::fr::NodeParameters& obj)
{
    Node node;
    node["pAllowHaltDueToClock"] = obj.pAllowHaltDueToClock;
    node["pAllowPassiveToActive"] = obj.pAllowPassiveToActive;
    node["pChannels"] = obj.pChannels;
    node["pClusterDriftDamping"] = obj.pClusterDriftDamping;
    node["pdAcceptedStartupRange"] = obj.pdAcceptedStartupRange;
    node["pdListenTimeout"] = obj.pdListenTimeout;
    node["pKeySlotId"] = obj.pKeySlotId;
    node["pKeySlotOnlyEnabled"] = obj.pKeySlotOnlyEnabled;
    node["pKeySlotUsedForStartup"] = obj.pKeySlotUsedForStartup;
    node["pKeySlotUsedForSync"] = obj.pKeySlotUsedForSync;
    node["pLatestTx"] = obj.pLatestTx;
    node["pMacroInitialOffsetA"] = obj.pMacroInitialOffsetA;
    node["pMacroInitialOffsetB"] = obj.pMacroInitialOffsetB;
    node["pMicroInitialOffsetA"] = obj.pMicroInitialOffsetA;
    node["pMicroInitialOffsetB"] = obj.pMicroInitialOffsetB;
    node["pMicroPerCycle"] = obj.pMicroPerCycle;
    node["pOffsetCorrectionOut"] = obj.pOffsetCorrectionOut;
    node["pOffsetCorrectionStart"] = obj.pOffsetCorrectionStart;
    node["pRateCorrectionOut"] = obj.pRateCorrectionOut;
    node["pWakeupChannel"] = obj.pWakeupChannel;
    node["pWakeupPattern"] = obj.pWakeupPattern;
    node["pdMicrotick"] = obj.pdMicrotick;
    node["pSamplesPerMicrotick"] = obj.pSamplesPerMicrotick;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::NodeParameters& obj)
{
    obj.pAllowHaltDueToClock = node["pAllowHaltDueToClock"].as<int>();
    obj.pAllowPassiveToActive = node["pAllowPassiveToActive"].as<int>();
    obj.pChannels = node["pChannels"].as<sim::fr::Channel>();
    obj.pClusterDriftDamping = node["pClusterDriftDamping"].as<int>();
    obj.pdAcceptedStartupRange = node["pdAcceptedStartupRange"].as<int>();
    obj.pdListenTimeout = node["pdListenTimeout"].as<int>();
    obj.pKeySlotId = node["pKeySlotId"].as<int>();
    obj.pKeySlotOnlyEnabled = node["pKeySlotOnlyEnabled"].as<int>();
    obj.pKeySlotUsedForStartup = node["pKeySlotUsedForStartup"].as<int>();
    obj.pKeySlotUsedForSync = node["pKeySlotUsedForSync"].as<int>();
    obj.pLatestTx = node["pLatestTx"].as<int>();
    obj.pMacroInitialOffsetA = node["pMacroInitialOffsetA"].as<int>();
    obj.pMacroInitialOffsetB = node["pMacroInitialOffsetB"].as<int>();
    obj.pMicroInitialOffsetA = node["pMicroInitialOffsetA"].as<int>();
    obj.pMicroInitialOffsetB = node["pMicroInitialOffsetB"].as<int>();
    obj.pMicroPerCycle = node["pMicroPerCycle"].as<int>();
    obj.pOffsetCorrectionOut = node["pOffsetCorrectionOut"].as<int>();
    obj.pOffsetCorrectionStart = node["pOffsetCorrectionStart"].as<int>();
    obj.pRateCorrectionOut = node["pRateCorrectionOut"].as<int>();
    obj.pWakeupChannel = node["pWakeupChannel"].as<sim::fr::Channel>();
    obj.pWakeupPattern = node["pWakeupPattern"].as<int>();
    obj.pdMicrotick = node["pdMicrotick"].as<sim::fr::ClockPeriod>();
    obj.pSamplesPerMicrotick = node["pSamplesPerMicrotick"].as<int>();
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::TransmissionMode& obj)
{
    Node node;

    switch (obj)
    {
    case sim::fr::TransmissionMode::Continuous:
        node = "Continuous";
        break;
    case sim::fr::TransmissionMode::SingleShot:
        node =  "SingleShot";
        break;
    default:
        throw Misconfiguration{ "Unknown TransmissionMode in VibConbersion::encode<TransmissionMode>" };
    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::TransmissionMode& obj)
{

    auto&& str = node.as<std::string>();
    if (str == "Continuous")
        obj = sim::fr::TransmissionMode::Continuous;
    else if (str == "SingleShot")
        obj = sim::fr::TransmissionMode::SingleShot;
    else
        return false;
    return true;

}

template<>
Node VibConversion::encode(const sim::fr::Channel& obj)
{
    Node node;

    switch (obj)
    {
    case sim::fr::Channel::A:
        node = "A";
        break;
    case sim::fr::Channel::B:
        node = "B";
        break;
    case sim::fr::Channel::AB:
        node =  "AB";
        break;
    case sim::fr::Channel::None:
        node = "None";
        break;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::Channel& obj)
{
    auto&& str = node.as<std::string>();
    if (str == "A")
        obj =  sim::fr::Channel::A;
    if (str == "B")
        obj =  sim::fr::Channel::B;
    if (str == "AB")
        obj =  sim::fr::Channel::AB;
    if (str == "None" || str == "")
        obj = sim::fr::Channel::None;
    else
        return false;
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::ClockPeriod& obj)
{
    Node node;
    switch (obj)
    {
    case sim::fr::ClockPeriod::T12_5NS:
        node = "12.5ns";
        break;
    case sim::fr::ClockPeriod::T25NS:
        node =  "25ns";
        break;
    case sim::fr::ClockPeriod::T50NS:
        node = "50ns";
        break;
    default:
        throw Misconfiguration{ "Unknown ClockPeriod in VibConversion::encode<ClockPeriod>" };

    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::ClockPeriod& obj)
{
    auto&& str = node.as<std::string>();
    if (str == "12.5ns")
        obj = sim::fr::ClockPeriod::T12_5NS;
    else if (str == "25ns")
        obj = sim::fr::ClockPeriod::T25NS;
    else if (str == "50ns")
        obj = sim::fr::ClockPeriod::T50NS;
    else
        return false;
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::TxBufferConfig& obj)
{
    Node node;
    node["channels"] = obj.channels;
    node["slotId"] = obj.slotId;
    node["offset"] = obj.offset;
    node["repetition"] = obj.repetition;
    node["PPindicator"] = obj.hasPayloadPreambleIndicator;
    node["headerCrc"] = obj.headerCrc;
    node["transmissionMode"] = obj.transmissionMode;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::TxBufferConfig& obj)
{
    obj.channels = node["channels"].as<sim::fr::Channel>();
    obj.slotId = node["slotId"].as<int>();
    obj.offset = node["offset"].as<int>();
    obj.repetition = node["repetition"].as<int>();
    obj.hasPayloadPreambleIndicator = node["PPindicator"].as<bool>();
    obj.headerCrc = node["headerCrc"].as<int>();
    obj.transmissionMode = node["transmissionMode"].as<sim::fr::TransmissionMode>();
    return true;
}

template<>
Node VibConversion::encode(const FlexrayController& obj)
{
    Node node;
    static const FlexrayController defaultObj;

    node["Name"] = obj.name;
    node["ClusterParameters"] = obj.clusterParameters;
    node["NodeParameters"] = obj.nodeParameters;
    node["TxBufferConfigs"] = obj.txBufferConfigs;
    non_default_encode(obj.useTraceSinks, node, "UseTraceSinks", defaultObj.useTraceSinks);
    if (!obj.useTraceSinks.empty())
    {
        node["Replay"] = obj.replay;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FlexrayController& obj)
{
    // backward compatibility to old json config files with only name of controller
    if (node.IsScalar())
    {
        obj.name = node.as<std::string>();
        return true;
    }

    obj.name = node["Name"].as<std::string>();
    optional_decode(obj.clusterParameters, node, "ClusterParameters");
    optional_decode(obj.nodeParameters, node, "NodeParameters");
    optional_decode(obj.txBufferConfigs, node, "TxBufferConfigs");
    if (node["UseTraceSinks"])
    {
        obj.useTraceSinks = node["UseTraceSinks"].as<decltype(obj.useTraceSinks)>();
    }
    optional_decode(obj.replay, node, "Replay");

    return true;
}

template<>
Node VibConversion::encode(const DigitalIoPort& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, DigitalIoPort& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const AnalogIoPort& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, AnalogIoPort& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const PwmPort& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, PwmPort& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const PatternPort& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, PatternPort& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const GenericPort& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, GenericPort& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const SyncType& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, SyncType& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Participant& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Participant& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Switch::Port& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Switch::Port& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Switch& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Switch& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Link& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Link& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const NetworkSimulator& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, NetworkSimulator& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TimeSync::SyncPolicy& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TimeSync::SyncPolicy& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TimeSync& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TimeSync& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const SimulationSetup& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, SimulationSetup& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const FastRtps::DiscoveryType& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FastRtps::DiscoveryType& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const FastRtps::Config& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FastRtps::Config& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const VAsio::RegistryConfig& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, VAsio::RegistryConfig& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const VAsio::Config& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, VAsio::Config& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const MiddlewareConfig& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, MiddlewareConfig& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const ExtensionConfig& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, ExtensionConfig& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Config& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Config& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TraceSink& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSink& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TraceSink::Type& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSink::Type& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TraceSource& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSource& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const TraceSource::Type& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSource::Type& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Replay& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Replay& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const Replay::Direction& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Replay::Direction& obj)
{
    return false;
}

} // namespace YAML
