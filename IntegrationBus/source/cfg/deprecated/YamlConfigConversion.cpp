// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfigConversion.hpp"

#include <algorithm>
#include <sstream>
#include <thread>
#include <chrono>

#include "ib/cfg/OptionalCfg.hpp"
#include "ib/cfg/string_utils.hpp"


using namespace ib::cfg;
using namespace ib;

//local utilities
namespace {

class BadVibConversion : public YAML::BadConversion
{
public:
    BadVibConversion(const YAML::Node& node, const std::string& message)
        :BadConversion(node.Mark())
    {
        msg = message;
    }
};

// Helper to parse a node as the given type or throw our BadVibConversion with the type's name in the error message.
template<typename T> struct ParseTypeName { static constexpr const char* name = "Unknown Type"; };

template<typename T> struct ParseTypeName<std::vector<T>> { static constexpr const char* name = ParseTypeName<T>::name; };

#define VIB_DECLARE_PARSE_TYPE_NAME(TYPE) \
    template<> struct ParseTypeName<TYPE> { static constexpr const char* name = #TYPE ; }

VIB_DECLARE_PARSE_TYPE_NAME(int16_t);
VIB_DECLARE_PARSE_TYPE_NAME(uint16_t);
VIB_DECLARE_PARSE_TYPE_NAME(uint64_t);
VIB_DECLARE_PARSE_TYPE_NAME(int64_t);
VIB_DECLARE_PARSE_TYPE_NAME(int8_t);
VIB_DECLARE_PARSE_TYPE_NAME(uint8_t);
VIB_DECLARE_PARSE_TYPE_NAME(int);
VIB_DECLARE_PARSE_TYPE_NAME(double);
VIB_DECLARE_PARSE_TYPE_NAME(bool);
VIB_DECLARE_PARSE_TYPE_NAME(std::vector<std::string>);
VIB_DECLARE_PARSE_TYPE_NAME(std::string);
VIB_DECLARE_PARSE_TYPE_NAME(MdfChannel);
VIB_DECLARE_PARSE_TYPE_NAME(Version);
VIB_DECLARE_PARSE_TYPE_NAME(Sink::Type);
VIB_DECLARE_PARSE_TYPE_NAME(ib::mw::logging::Level);
VIB_DECLARE_PARSE_TYPE_NAME(Sink);
VIB_DECLARE_PARSE_TYPE_NAME(Logger);
VIB_DECLARE_PARSE_TYPE_NAME(CanController);
VIB_DECLARE_PARSE_TYPE_NAME(LinController);
VIB_DECLARE_PARSE_TYPE_NAME(EthernetController);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::ClusterParameters);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::NodeParameters);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::TxBufferConfig);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::Channel);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::ClockPeriod);
VIB_DECLARE_PARSE_TYPE_NAME(ib::sim::fr::TransmissionMode);
VIB_DECLARE_PARSE_TYPE_NAME(FlexrayController);
VIB_DECLARE_PARSE_TYPE_NAME(DigitalIoPort);
VIB_DECLARE_PARSE_TYPE_NAME(AnalogIoPort);
VIB_DECLARE_PARSE_TYPE_NAME(PwmPort);
VIB_DECLARE_PARSE_TYPE_NAME(PatternPort);
VIB_DECLARE_PARSE_TYPE_NAME(GenericPort);
VIB_DECLARE_PARSE_TYPE_NAME(GenericPort::ProtocolType);
VIB_DECLARE_PARSE_TYPE_NAME(DataPort);
VIB_DECLARE_PARSE_TYPE_NAME(RpcPort);
VIB_DECLARE_PARSE_TYPE_NAME(SyncType);
VIB_DECLARE_PARSE_TYPE_NAME(std::chrono::milliseconds);
VIB_DECLARE_PARSE_TYPE_NAME(std::chrono::nanoseconds);
VIB_DECLARE_PARSE_TYPE_NAME(ParticipantController);
VIB_DECLARE_PARSE_TYPE_NAME(Participant);
VIB_DECLARE_PARSE_TYPE_NAME(Switch::Port);
VIB_DECLARE_PARSE_TYPE_NAME(Switch);
VIB_DECLARE_PARSE_TYPE_NAME(Link);
VIB_DECLARE_PARSE_TYPE_NAME(NetworkSimulator);
VIB_DECLARE_PARSE_TYPE_NAME(TimeSync::SyncPolicy);
VIB_DECLARE_PARSE_TYPE_NAME(TimeSync);
VIB_DECLARE_PARSE_TYPE_NAME(SimulationSetup);
VIB_DECLARE_PARSE_TYPE_NAME(FastRtps::DiscoveryType);
VIB_DECLARE_PARSE_TYPE_NAME(FastRtps::Config);
VIB_DECLARE_PARSE_TYPE_NAME(VAsio::RegistryConfig);
VIB_DECLARE_PARSE_TYPE_NAME(VAsio::Config);
VIB_DECLARE_PARSE_TYPE_NAME(Middleware);
VIB_DECLARE_PARSE_TYPE_NAME(MiddlewareConfig);
VIB_DECLARE_PARSE_TYPE_NAME(ExtensionConfig);
VIB_DECLARE_PARSE_TYPE_NAME(Config);
VIB_DECLARE_PARSE_TYPE_NAME(TraceSink);
VIB_DECLARE_PARSE_TYPE_NAME(TraceSink::Type);
VIB_DECLARE_PARSE_TYPE_NAME(TraceSource);
VIB_DECLARE_PARSE_TYPE_NAME(TraceSource::Type);
VIB_DECLARE_PARSE_TYPE_NAME(Replay);
VIB_DECLARE_PARSE_TYPE_NAME(Replay::Direction);

#undef VIB_DECLARE_PARSE_TYPE_NAME

template<typename ValueT>
auto parse_as(const YAML::Node& node) -> ValueT
{
    try
    {
        return node.as<ValueT>();
    }
    catch(const BadVibConversion&)
    {
        //we already have a concise error message, propagate it to our caller
        throw;
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "Cannot parse as Type \"" << ParseTypeName<ValueT>::name
            << "\". Exception: \"" << ex.what()
            << "\". While parsing: " << YAML::Dump(node);
        throw BadVibConversion(node, ss.str());
    }
}

inline auto nibble_to_char(char nibble) -> char
{
    nibble &= 0xf;
    if (0 <= nibble && nibble < 10)
        return '0' + nibble;
    else
        return 'a' + (nibble - 10);
}

inline auto char_to_nibble(char c) -> char
{
    if (c < '0' || c > 'f')
        throw std::runtime_error("OutOfRange");


    if (c < 'a')
        return c - '0';
    else
        return c - 'a' + 10;
}

auto hex_encode(const std::vector<uint8_t>& data) -> std::string
{
    std::stringstream out;
    for (auto&& byte : data)
    {
        out << nibble_to_char(byte >> 4)
            << nibble_to_char(byte);
    }
    return out.str();
}

auto hex_decode(const std::string& str) -> std::vector<uint8_t>
{
    if (str.size() % 2 != 0)
        throw std::runtime_error("InvalidStrFormat");

    std::vector<uint8_t> result;
    result.reserve(str.size() / 2);

    for (auto iter = str.begin(); iter != str.end(); iter += 2)
    {
        char high = char_to_nibble(*iter);
        char low = char_to_nibble(*(iter + 1));

        result.push_back(high << 4 | low);
    }
    return result;
}

auto macaddress_encode(const std::array<uint8_t, 6>& macAddress) -> YAML::Node
{
    std::stringstream macOut;

    to_ostream(macOut, macAddress);

    return YAML::Node(macOut.str());
}

auto macaddress_decode(const YAML::Node& node) -> std::array<uint8_t, 6>
{
    std::array<uint8_t, 6> macAddress;

    std::stringstream macIn(parse_as<std::string>(node));
    from_istream(macIn, macAddress);

    return macAddress;
}

template<typename ConfigT>
void optional_encode(const OptionalCfg<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.has_value())
    {
        node[fieldName] = value.value();
    }
}
template<typename ConfigT>
void optional_encode(const std::vector<ConfigT>& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.size() > 0)
    {
        node[fieldName] = value;
    }
}

void optional_encode(const Replay& value, YAML::Node& node, const std::string& fieldName)
{
    if (value.useTraceSource.size() > 0)
    {
        node[fieldName] = value;
    }
}

template<typename ConfigT>
void optional_decode(OptionalCfg<ConfigT>& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

template<typename ConfigT>
void optional_decode(ConfigT& value, const YAML::Node& node, const std::string& fieldName)
{
    if (node.IsMap() && node[fieldName]) //operator[] does not modify node
    {
        value = parse_as<ConfigT>(node[fieldName]);
    }
}

//!< We support some legacy controller definitions which only consisted of a single
//   'string' valued node specifying a name. Currently an object with an "Name" field
//   must be specified.
//   Legacy format for: Can, Lin, FlexRay, IO Ports, GenericMessage.
// Returns true if legacy format is used
template<typename ControllerT>
bool legacy_decode_name(ControllerT& value, const YAML::Node& node)
{
    if (node.IsScalar())
    {
        //legacy format: Controller/Node/Subscriber as: ["name1", "name2"...]
        value.name = parse_as<std::string>(node);
        return true;
    }
    if (node["Name"])
    {
        // new style object with fields
        value.name = parse_as<std::string>(node["Name"]);
        return false;
    }
    else
    {
        // legacy format, e.g; "CanControllers": ["CAN1", ...]
        if (node.IsMap())
        {
            auto it = node.begin();
            if (it == node.end())
            {
                std::stringstream ss;
                ss << "Cannot parse as Type \"" << ParseTypeName<ControllerT>::name
                    << "\". While parsing: " << YAML::Dump(node);
                throw BadVibConversion(node, ss.str());
            }
            value.name = parse_as<std::string>(it->first);
        }
        return true;
    }
}

void legacy_decode_networksimulator(SimulationSetup& simulationSetup, const YAML::Node& node)
{
    // Legacy NetworkSimulator configuration:
    // The NetworkSimulators config was located in the SimulationSetup config.
    // It now resides in the Participant config.
    // We'll handle the legacy case as follows:
    // if there is a NetworkSimulators block, move the definition to a participant
    // which declares the use of this network simulator.

    auto findNetSimUser = [&](const auto& name) -> Participant* {
        for (auto& participant : simulationSetup.participants)
        {
            for (const auto& usedNetSim : participant.networkSimulators)
            {
                if (usedNetSim.name == name)
                {
                    return  &participant;
                }
            }
        }
        return nullptr;
    };
    std::vector<NetworkSimulator> netSims;
    optional_decode(netSims, node, "NetworkSimulators");
    for (const auto& netSim : netSims)
    {
        auto* participant = findNetSimUser(netSim.name);
        if (!participant)
        {
            continue;
        }
        // copy the definition to the participant that refers
        // to the network simulator by name
        participant->networkSimulators = netSims;
    }

}


template <typename ConfigT>
auto non_default_encode(const std::vector<ConfigT>& values, YAML::Node& node, const std::string& fieldName, const std::vector<ConfigT>& defaultValue)
{
    // Only encode vectors that have members that deviate from a default-value.
    // And also ensure we only encode values that are user-defined.
    if (!values.empty() && !(values == defaultValue))
    {
        std::vector<ConfigT> userValues;  //XXX must not encode uint8/uint16 as character!, need upgrade ConfigT to uint32_t here magically
        static const ConfigT defaultObj{};
        // only encode non-default values
        for (const auto& value : values)
        {
            if (!(value == defaultObj))
            {
                userValues.push_back(value);
            }
        }
        if (userValues.size() > 0)
        {
            node[fieldName] = values;
        }
    }
}

template <typename ConfigT>
auto non_default_encode(const ConfigT& value, YAML::Node& node, const std::string& fieldName, const ConfigT& defaultValue)
{
    if (!(value == defaultValue))
        node[fieldName] = value;
}


} //end anonymous

// YAML type conversion helpers for our data types.
// Ported from the JsonConfig from_json/to_json templates to the
// internal conversion mechanism of yaml-cpp.
namespace YAML {


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
        throw BadVibConversion(node, "MdfChannel should be a Map");
    }
    optional_decode(obj.channelName, node, "ChannelName");
    optional_decode(obj.channelPath, node, "ChannelPath");
    optional_decode(obj.channelSource, node, "ChannelSource");
    optional_decode(obj.groupName, node, "GroupName");
    optional_decode(obj.groupPath, node, "GroupPath");
    optional_decode(obj.groupSource, node, "GroupSource");
    return true;
}

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
        throw BadVibConversion(node, "Version must be a string \"x.y.z\"");
    }
    std::stringstream in(parse_as<std::string>(node));
    in >> obj;
    if (in.fail())
    {
        throw BadVibConversion(node, "Version must be a string \"x.y.z\"");
    }
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
        throw BadVibConversion(node, "Sink::Type should be a string of Remote|Stdout|File.");
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
        throw BadVibConversion(node, "Unknown Sink::Type: " + str + ".");
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
    if (!node.IsScalar())
    {
        throw BadVibConversion(node, 
            "Level should be a string of Critical|Error|Warn|Info|Debug|Trace|Off.");
    }
    auto&& str = parse_as<std::string>(node);
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
    {
        throw BadVibConversion(node, "Unknown mw::logging::Level: " + str + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const Sink& obj)
{
    static const Sink defaultSink{};
    Node node;
    // IbConfig.schema.json: Type is required:
    node["Type"] = obj.type;
    non_default_encode(obj.level, node, "Level", defaultSink.level);
    non_default_encode(obj.logname, node, "Logname", defaultSink.logname);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Sink& obj)
{
    optional_decode(obj.type, node, "Type");
    optional_decode(obj.level, node, "Level");

    if (obj.type == Sink::Type::File)
    {
        if (!node["Logname"])
        {
            throw BadVibConversion(node, "Sink of type Sink::Type::File requires a Logname");
        }
        obj.logname = parse_as<std::string>(node["Logname"]);
    }

    return true;
}

template<>
Node VibConversion::encode(const Logger& obj)
{
    Node node;
    static const Logger defaultLogger{};

    non_default_encode(obj.logFromRemotes, node, "LogFromRemotes", defaultLogger.logFromRemotes);
    non_default_encode(obj.flush_level, node, "FlushLevel", defaultLogger.flush_level);
    // IbConfig.schema.json: this is a required property:
    node["Sinks"] = obj.sinks;

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
    Node node;
    node["Name"] = obj.name;
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, CanController& obj)
{
    legacy_decode_name(obj, node);
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const LinController& obj)
{
    Node node;
    node["Name"] = obj.name;

    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, LinController& obj)
{
    legacy_decode_name(obj, node);
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const EthernetController& obj)
{
    Node node;
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

    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");

    return node;
}
template<>
bool VibConversion::decode(const Node& node, EthernetController& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.macAddress = macaddress_decode(node["MacAddr"]);
    optional_decode(obj.pcapFile, node, "PcapFile");
    optional_decode(obj.pcapPipe, node, "PcapPipe");
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::ClusterParameters& obj)
{
    Node node;

    //NB we have to ensure parsing as integer, uint8_t will be interpreted as character
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
template<>
bool VibConversion::decode(const Node& node, sim::fr::ClusterParameters& obj)
{
    //NB parsing with as<uint8_t>() leads to problems
    auto parseInt = [&node](auto instance, auto name)
    {
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
    obj.gdSymbolWindowActionPointOffset = parseInt(obj.gdSymbolWindowActionPointOffset, "gdSymbolWindowActionPointOffset");
    obj.gdTSSTransmitter = parseInt(obj.gdTSSTransmitter, "gdTSSTransmitter");
    obj.gdWakeupTxActive = parseInt(obj.gdWakeupTxActive, "gdWakeupTxActive");
    obj.gdWakeupTxIdle = parseInt(obj.gdWakeupTxIdle, "gdWakeupTxIdle");
    obj.gListenNoise = parseInt(obj.gListenNoise, "gListenNoise");
    obj.gMacroPerCycle = parseInt(obj.gMacroPerCycle, "gMacroPerCycle");
    obj.gMaxWithoutClockCorrectionFatal = parseInt(obj.gMaxWithoutClockCorrectionFatal, "gMaxWithoutClockCorrectionFatal");
    obj.gMaxWithoutClockCorrectionPassive = parseInt(obj.gMaxWithoutClockCorrectionPassive, "gMaxWithoutClockCorrectionPassive");
    obj.gNumberOfMiniSlots = parseInt(obj.gNumberOfMiniSlots, "gNumberOfMiniSlots");
    obj.gNumberOfStaticSlots = parseInt(obj.gNumberOfStaticSlots, "gNumberOfStaticSlots");
    obj.gPayloadLengthStatic = parseInt(obj.gPayloadLengthStatic, "gPayloadLengthStatic");
    obj.gSyncFrameIDCountMax = parseInt(obj.gSyncFrameIDCountMax, "gSyncFrameIDCountMax");
    return true;
}
template<>
Node VibConversion::encode(const sim::fr::NodeParameters& obj)
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
template<>
bool VibConversion::decode(const Node& node, sim::fr::NodeParameters& obj)
{
    auto parseInt = [&node](auto instance, auto name)
    {
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
    obj.pWakeupChannel = parse_as<sim::fr::Channel>(node["pWakeupChannel"]);
    obj.pdMicrotick = parse_as<sim::fr::ClockPeriod>(node["pdMicrotick"]);
    obj.pChannels = parse_as<sim::fr::Channel>(node["pChannels"]);
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
        throw BadVibConversion(node, "Unknown TransmissionMode in VibConbersion::encode<TransmissionMode>");
    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::TransmissionMode& obj)
{

    auto&& str = parse_as<std::string>(node);
    if (str == "Continuous")
        obj = sim::fr::TransmissionMode::Continuous;
    else if (str == "SingleShot")
        obj = sim::fr::TransmissionMode::SingleShot;
    else
    {
        throw BadVibConversion(node, "Unknown sim::fr::TransmissionMode: " + str);
    }
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
    auto&& str = parse_as<std::string>(node);
    if (str == "A")
        obj =  sim::fr::Channel::A;
    else if (str == "B")
        obj =  sim::fr::Channel::B;
    else if (str == "AB")
        obj =  sim::fr::Channel::AB;
    else if (str == "None" || str == "")
        obj = sim::fr::Channel::None;
    else
    {
        throw BadVibConversion(node, "Unknown sim::fr::Channel: " + str);
    }
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
        throw BadVibConversion(node, "Unknown sim::fr::ClockPeriod in VibConversion::encode<ClockPeriod>");

    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, sim::fr::ClockPeriod& obj)
{
    auto&& str = parse_as<std::string>(node);
    if (str == "12.5ns")
        obj = sim::fr::ClockPeriod::T12_5NS;
    else if (str == "25ns")
        obj = sim::fr::ClockPeriod::T25NS;
    else if (str == "50ns")
        obj = sim::fr::ClockPeriod::T50NS;
    else
    {
        throw BadVibConversion(node, "Unknown sim::fr::ClockPeriod: " + str);
    }
    return true;
}

template<>
Node VibConversion::encode(const sim::fr::TxBufferConfig& obj)
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
template<>
bool VibConversion::decode(const Node& node, sim::fr::TxBufferConfig& obj)
{
    obj.channels = parse_as<sim::fr::Channel>(node["channels"]);
    obj.slotId = parse_as<uint16_t>(node["slotId"]);
    obj.offset = static_cast<uint8_t>(parse_as<int>(node["offset"]));
    obj.repetition = static_cast<uint8_t>(parse_as<int>(node["repetition"]));
    obj.hasPayloadPreambleIndicator = parse_as<bool>(node["PPindicator"]);
    obj.headerCrc = parse_as<uint16_t>(node["headerCrc"]);
    obj.transmissionMode = parse_as<sim::fr::TransmissionMode>(node["transmissionMode"]);
    return true;
}

template<>
Node VibConversion::encode(const FlexrayController& obj)
{
    static const FlexrayController defaultObj{};
    Node node;

    node["Name"] = obj.name;
    non_default_encode(obj.clusterParameters, node, "ClusterParameters", defaultObj.clusterParameters);
    non_default_encode(obj.nodeParameters, node, "NodeParameters", defaultObj.nodeParameters);
    non_default_encode(obj.txBufferConfigs, node, "TxBufferConfigs", defaultObj.txBufferConfigs);
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FlexrayController& obj)
{
    legacy_decode_name(obj, node);
    optional_decode(obj.clusterParameters, node, "ClusterParameters");
    optional_decode(obj.nodeParameters, node, "NodeParameters");
    optional_decode(obj.txBufferConfigs, node, "TxBufferConfigs");
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");

    return true;
}

template<>
Node VibConversion::encode(const DigitalIoPort& obj)
{
    Node node;
    node["Name"] = obj.name;
    if (obj.direction == PortDirection::Out)
    {
        node["value"] = obj.initvalue;
    }

    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, DigitalIoPort& obj)
{
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    optional_decode(obj.initvalue, node, "value"); //only for Digital-In ports, non-strict

    if (legacy_decode_name(obj, node))
    {
        // legacy format: {"portName" : bool}
        if (node.IsMap()) {
            auto it = node.begin();
            if (it != node.end() && it->second.IsScalar())
            {
                auto val = it->second.as<bool>();
                obj.initvalue = val;
            }
        }
    }

    return true;
}

template<>
Node VibConversion::encode(const AnalogIoPort& obj)
{
    Node node;
    node["Name"] = obj.name;
    if (obj.direction == PortDirection::Out)
    {
        node["value"] = obj.initvalue;
        node["unit"] = obj.unit;
    }

    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");

    return node;
}
template<>
bool VibConversion::decode(const Node& node, AnalogIoPort& obj)
{
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");

    if (legacy_decode_name(obj, node))
    {
        //legacy format {"name" : {"value" : number, "unit" : str}}
        auto it = node.begin();
        if (it != node.end() && it->second.IsMap())
        {
            const auto& valUnit = it->second;
            obj.initvalue = parse_as<double>(valUnit["value"]);
            obj.unit = parse_as<std::string>(valUnit["unit"]);
        }
        return true;
    }

    if (node["value"])
    {
        obj.initvalue = parse_as<decltype(obj.initvalue)>(node["value"]);
        obj.unit = parse_as<std::string>(node["unit"]);
    }

    return true;
}

template<>
Node VibConversion::encode(const PwmPort& obj)
{
    Node node;
    node["Name"] = obj.name;
    if (obj.direction == PortDirection::Out)
    {
        Node freq;
        freq["value"] = obj.initvalue.frequency;
        freq["unit"] = obj.unit;
        node["freq"] = freq;
        node["duty"] = obj.initvalue.dutyCycle;
    }

    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, PwmPort& obj)
{
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");

    if (legacy_decode_name(obj, node))
    {
        // legacy format: {"portName" : {"value": double, "unit": "str"}, "duty":double}}
        auto it = node.begin();
        if (it != node.end() && it->second.IsMap())
        {
            const auto& val = it->second;
            obj.initvalue.frequency = parse_as<double>(val["freq"]["value"]);
            obj.unit = parse_as<std::string>(val["freq"]["unit"]);
            obj.initvalue.dutyCycle = parse_as<double>(val["duty"]);
        }
        return true;
    }

    if (node["freq"]) //only given for Pwm-Out
    {
        obj.initvalue.frequency = parse_as<double>(node["freq"]["value"]);
        obj.unit = parse_as<std::string>(node["freq"]["unit"]);
        obj.initvalue.dutyCycle = parse_as<double>(node["duty"]);
    }
    return true;
}

template<>
Node VibConversion::encode(const PatternPort& obj)
{
    Node node;
    node["Name"] = obj.name;
    if (obj.direction == PortDirection::Out)
    {
        node["value"] = hex_encode(obj.initvalue);
    }
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, PatternPort& obj)
{
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");

    if (legacy_decode_name(obj, node))
    {
        //legacy format: {"name" : "hexval"}
        auto it = node.begin();
        if (it != node.end() && it->second.IsScalar())
        {
            obj.initvalue = hex_decode(parse_as<std::string>(it->second));
        }
        return true;
    }

    if (node["value"])
    {
        obj.initvalue = hex_decode(parse_as<std::string>(node["value"]));
    }
    return true;
}

template<>
Node VibConversion::encode(const GenericPort& obj)
{
    static const GenericPort defaultObj{};
    Node node;
    node["Name"] = obj.name;
    non_default_encode(obj.protocolType, node, "Protocol", defaultObj.protocolType);
    non_default_encode(obj.definitionUri, node, "DefinitionUri", defaultObj.definitionUri);
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, GenericPort& obj)
{
    // current format: {"Name": "obj.name", ...}
    optional_decode(obj.protocolType, node, "Protocol");
    optional_decode(obj.definitionUri, node, "DefinitionUri");
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");

    if (legacy_decode_name(obj, node))
    {
        //legacy format: Subscribers as: ["subscriber1", "subscriber2"...]
        // only name required
    }

    return true;
}

template<>
Node VibConversion::encode(const GenericPort::ProtocolType& obj)
{
    Node node;
    switch (obj)
    {
    case GenericPort::ProtocolType::ROS:
        node = "ROS";
        break;
    case GenericPort::ProtocolType::SOMEIP:
        node = "SOME/IP";
        break;
    case GenericPort::ProtocolType::Undefined:
        node = "Undefined";
        break;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, GenericPort::ProtocolType& obj)
{

    auto&& str = parse_as<std::string>(node);
    if (str == "ROS")
        obj = GenericPort::ProtocolType::ROS;
    else if (str == "SOME/IP")
        obj = GenericPort::ProtocolType::SOMEIP;
    else if (str == "Undefined")
        obj = GenericPort::ProtocolType::Undefined;
    else if (str == "")
        obj = GenericPort::ProtocolType::Undefined;
    else
    {
        throw BadVibConversion(node, "Unknown GenericPort::ProtoclType: " + str + ".");
    }
    return true;
}

template <>
Node VibConversion::encode(const DataPort& obj)
{
    static const DataPort defaultObj{};
    Node node;
    node["Name"] = obj.name;
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template <>
bool VibConversion::decode(const Node& node, DataPort& obj)
{
    // current format: {"Name": "obj.name", ...}
    legacy_decode_name(obj, node);
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template <>
Node VibConversion::encode(const RpcPort& obj)
{
    static const RpcPort defaultObj{};
    Node node;
    node["Name"] = obj.name;
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template <>
bool VibConversion::decode(const Node& node, RpcPort& obj)
{
    // current format: {"Name": "obj.name", ...}
    legacy_decode_name(obj, node);
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const SyncType& obj)
{
    Node node;
    node = to_string(obj);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, SyncType& obj)
{
    auto&& syncType = parse_as<std::string>(node);

    if (syncType == "DistributedTimeQuantum")
        obj = SyncType::DistributedTimeQuantum;
    else if (syncType == "DiscreteEvent")
        obj = SyncType::DiscreteEvent;
    else if (syncType == "TimeQuantum")
        obj = SyncType::TimeQuantum;
    else if (syncType == "DiscreteTime")
        obj = SyncType::DiscreteTime;
    else if (syncType == "DiscreteTimePassive")
        obj = SyncType::DiscreteTimePassive;
    else if (syncType == "Unsynchronized")
        obj = SyncType::Unsynchronized;
    else
    {
        throw BadVibConversion(node, "Unknown SyncType: " + syncType + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const ParticipantController& obj)
{

    Node node;
    node["SyncType"] = obj.syncType;
    auto optional_duration = [&node](auto name, auto x) {
        if (x != decltype(x)::max())
            node[name] = std::chrono::duration_cast<std::chrono::milliseconds>(x).count();
    };
    optional_duration("ExecTimeLimitSoftMs", obj.execTimeLimitSoft);
    optional_duration("ExecTimeLimitHardMs", obj.execTimeLimitHard);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, ParticipantController& obj)
{
    obj.syncType = parse_as<SyncType>(node["SyncType"]);
    optional_decode(obj.execTimeLimitHard, node, "ExecTimeLimitHardMs");
    optional_decode(obj.execTimeLimitSoft, node, "ExecTimeLimitSoftMs");
    return true;
}

template<>
Node VibConversion::encode(const std::chrono::milliseconds& obj)
{
    Node node;
    node = obj.count();
    return node;
}
template<>
bool VibConversion::decode(const Node& node, std::chrono::milliseconds& obj)
{
    obj = std::chrono::milliseconds{parse_as<uint64_t>(node)};
    return true;
}

template<>
Node VibConversion::encode(const std::chrono::nanoseconds& obj)
{
    Node node;
    node = obj.count();
    return node;
}
template<>
bool VibConversion::decode(const Node& node, std::chrono::nanoseconds& obj)
{
    obj = std::chrono::nanoseconds{parse_as<uint64_t>(node)};
    return true;
}

template<>
Node VibConversion::encode(const Participant& obj)
{
    static const Participant defaultObj{};
    Node node;

    auto makePortList = [&node](auto&& portVector, PortDirection direction, const auto& name)
    {

        using PortList = typename std::decay_t<decltype(portVector)>;
        PortList sequence;
        for (auto&& port : portVector)
        {
            if (port.direction == direction)
            {
                sequence.push_back(port);
            }
        }
        // only encode if user-defined ports are present
        if (sequence.size() > 0)
        {
            node[name] = sequence;
        }
    };

    // GenericSubscribers/DataSubscribers cannot be easily discerned from GenericPublishers/DataPublishers (same type of GenericPort/DataPort),
    // so we treat them specially here:
    auto makeSubscribers = [](auto parentNode, auto&& subscribers)
    {
        for (const auto& subscriber : subscribers)
        {
            YAML::Node subNode;
            subNode["Name"] = subscriber.name;
            optional_encode(subscriber.useTraceSinks, subNode, "UseTraceSinks");
            optional_encode(subscriber.replay, subNode, "Replay");
            parentNode.push_back(subNode);
        }
    };


    // RpcServers cannot be easily discerned from RpcClients (same type of RpcPort),
    // so we treat them specially here:
    auto makeRpcServers = [](auto parentNode, auto&& servers) {
        for (const auto& server : servers)
        {
            YAML::Node subNode;
            subNode["Name"] = server.name;
            optional_encode(server.useTraceSinks, subNode, "UseTraceSinks");
            optional_encode(server.replay, subNode, "Replay");
            parentNode.push_back(subNode);
        }
    };

    node["Name"] = obj.name;
    non_default_encode(obj.description, node, "Description", defaultObj.description);
    non_default_encode(obj.logger, node, "Logger", defaultObj.logger);

    optional_encode(obj.canControllers, node, "CanControllers");
    optional_encode(obj.linControllers, node, "LinControllers");
    optional_encode(obj.ethernetControllers, node, "EthernetControllers");
    optional_encode(obj.flexrayControllers, node, "FlexRayControllers");
    optional_encode(obj.networkSimulators, node, "NetworkSimulators");

    makePortList(obj.analogIoPorts, PortDirection::In, "Analog-In");
    makePortList(obj.digitalIoPorts, PortDirection::In, "Digital-In");
    makePortList(obj.pwmPorts, PortDirection::In, "Pwm-In");
    makePortList(obj.patternPorts, PortDirection::In, "Pattern-In");
    makePortList(obj.analogIoPorts, PortDirection::Out, "Analog-Out");
    makePortList(obj.digitalIoPorts, PortDirection::Out, "Digital-Out");
    makePortList(obj.pwmPorts, PortDirection::Out, "Pwm-Out");
    makePortList(obj.patternPorts, PortDirection::Out, "Pattern-Out");

    optional_encode(obj.genericPublishers, node, "GenericPublishers");
    makeSubscribers(node["GenericSubscribers"], obj.genericSubscribers);

    optional_encode(obj.dataPublishers, node, "DataPublishers");
    makeSubscribers(node["DataSubscribers"], obj.dataSubscribers);
	
    optional_encode(obj.rpcClients, node, "RpcClients");
    makeRpcServers(node["RpcServers"], obj.rpcServers);

    optional_encode(obj.traceSinks, node, "TraceSinks");
    optional_encode(obj.traceSources, node, "TraceSources");
    non_default_encode(obj.isSyncMaster, node, "IsSyncMaster", defaultObj.isSyncMaster);
    optional_encode(obj.participantController, node, "ParticipantController");

    return node;
}
template<>
bool VibConversion::decode(const Node& node, Participant& obj)
{
    // we need to explicitly set the direction for the parsed port
    auto makePorts = [&node](auto&& portList, auto&& propertyName, auto&& direction)
    {
        using PortList = typename std::decay_t<decltype(portList)>;
        using PortType = typename std::decay_t<decltype(portList)>::value_type;

        PortList ports;
        optional_decode(ports, node, propertyName);
        for (auto& port : ports) {
            port.direction = direction;
            portList.push_back(port);
        }
    };

    obj.name = parse_as<std::string>(node["Name"]);

    optional_decode(obj.description, node, "Description");
    optional_decode(obj.logger, node, "Logger");
    optional_decode(obj.isSyncMaster, node, "IsSyncMaster");
    optional_decode(obj.participantController, node, "ParticipantController");
    optional_decode(obj.canControllers, node, "CanControllers");
    optional_decode(obj.linControllers, node, "LinControllers");
    optional_decode(obj.ethernetControllers, node, "EthernetControllers");
    optional_decode(obj.flexrayControllers, node, "FlexRayControllers");
    optional_decode(obj.networkSimulators, node, "NetworkSimulators");

    optional_decode(obj.genericPublishers, node, "GenericPublishers");
    optional_decode(obj.genericSubscribers, node, "GenericSubscribers");

    optional_decode(obj.dataPublishers, node, "DataPublishers");
    optional_decode(obj.dataSubscribers, node, "DataSubscribers");
	
    optional_decode(obj.rpcClients, node, "RpcClients");
    optional_decode(obj.rpcServers, node, "RpcServers");

    optional_decode(obj.traceSinks, node, "TraceSinks");
    optional_decode(obj.traceSources, node, "TraceSources");

    makePorts(obj.digitalIoPorts, "Digital-Out", PortDirection::Out);
    makePorts(obj.digitalIoPorts, "Digital-In", PortDirection::In);
    makePorts(obj.analogIoPorts, "Analog-Out", PortDirection::Out);
    makePorts(obj.analogIoPorts, "Analog-In", PortDirection::In);
    makePorts(obj.pwmPorts, "Pwm-Out", PortDirection::Out);
    makePorts(obj.pwmPorts, "Pwm-In", PortDirection::In);
    makePorts(obj.patternPorts, "Pattern-Out", PortDirection::Out);
    makePorts(obj.patternPorts, "Pattern-In", PortDirection::In);


    return true;
}

template<>
Node VibConversion::encode(const Switch::Port& obj)
{
    static const Switch::Port defaultObj{};
    Node node;
    node["Name"] = obj.name;
    non_default_encode(obj.vlanIds, node, "VlanIds", defaultObj.vlanIds);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Switch::Port& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    optional_decode(obj.vlanIds, node, "VlanIds");
    return true;
}

template<>
Node VibConversion::encode(const Switch& obj)
{
    static const Switch defaultObj{};
    Node node;
    node["Name"] = obj.name;
    node["Ports"] = obj.ports;
    non_default_encode(obj.description, node, "Description", defaultObj.description);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Switch& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    optional_decode(obj.description, node, "Description");
    optional_decode(obj.ports, node, "Ports");
    return true;
}

template<>
Node VibConversion::encode(const Link& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["Endpoints"] = obj.endpoints;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Link& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.endpoints = parse_as<decltype(obj.endpoints)>(node["Endpoints"]);
    return true;
}

template<>
Node VibConversion::encode(const NetworkSimulator& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["SimulatedLinks"] = obj.simulatedLinks;
    node["SimulatedSwitches"] = obj.simulatedSwitches;
    optional_encode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_encode(obj.replay, node, "Replay");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, NetworkSimulator& obj)
{
    //legacy format: "nameOfNetSim"
    if (node.IsScalar())
    {
        // will be fixed up later by legacy_decode_networksimulator
        obj.name = parse_as<std::string>(node);
        return true;
    }
    obj.name = parse_as<std::string>(node["Name"]);
    optional_decode(obj.simulatedSwitches, node, "SimulatedSwitches");
    optional_decode(obj.simulatedLinks, node, "SimulatedLinks");
    optional_decode(obj.useTraceSinks, node, "UseTraceSinks");
    optional_decode(obj.replay, node, "Replay");
    return true;
}

template<>
Node VibConversion::encode(const TimeSync::SyncPolicy& obj)
{
    Node node;
    try
    {
        node = to_string(obj);
    }
    catch (const ib::type_conversion_error&)
    {
        node = "UNKNOWN_SYNC_POLICY";
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TimeSync::SyncPolicy& obj)
{
    auto&& syncType = parse_as<std::string>(node);
    if (syncType == "Loose")
        obj = TimeSync::SyncPolicy::Loose;
    else if (syncType == "Strict")
        obj = TimeSync::SyncPolicy::Strict;
    else
    {
        throw BadVibConversion(node, "Unknown TimeSync::SyncPolicy: " + syncType + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const TimeSync& obj)
{
    static const TimeSync defaultObj{};
    Node node;
    non_default_encode(obj.syncPolicy, node, "SyncPolicy", defaultObj.syncPolicy);
    node["TickPeriodNs"] = static_cast<uint64_t>(obj.tickPeriod.count());
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TimeSync& obj)
{
    optional_decode(obj.syncPolicy, node, "SyncPolicy");
    optional_decode(obj.tickPeriod, node, "TickPeriodNs");
    return true;
}

template<>
Node VibConversion::encode(const SimulationSetup& obj)
{
    static const SimulationSetup defaultObj;
    Node node;
    node["Participants"] = obj.participants;
    node["Links"] = obj.links;
    optional_encode(obj.switches, node, "Switches");
    non_default_encode(obj.timeSync, node, "TimeSync", defaultObj.timeSync);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, SimulationSetup& obj)
{
    obj.participants = parse_as<decltype(obj.participants)>(node["Participants"]);
    optional_decode(obj.links, node, "Links");
    optional_decode(obj.timeSync, node, "TimeSync");
    optional_decode(obj.switches, node, "Switches");

    // legacy format: moves network simulator to the participant reffering to it.
    legacy_decode_networksimulator(obj, node);
    return true;
}

template<>
Node VibConversion::encode(const FastRtps::DiscoveryType& obj)
{
    Node node;
    try
    {
        node = to_string(obj);
    }
    catch (const ib::type_conversion_error&)
    {
        node = "UNKNOWN_DISCOVERY_TYPE";
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FastRtps::DiscoveryType& obj)
{
    auto&& discoveryType = parse_as<std::string>(node);

    if (discoveryType == "Local")
        obj = FastRtps::DiscoveryType::Local;
    else if (discoveryType == "Multicast")
        obj = FastRtps::DiscoveryType::Multicast;
    else if (discoveryType == "Unicast")
        obj = FastRtps::DiscoveryType::Unicast;
    else if (discoveryType == "ConfigFile")
        obj = FastRtps::DiscoveryType::ConfigFile;
    else
    {
        throw BadVibConversion(node, "Unknown FastRtps::DiscoveryType: " + discoveryType + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const FastRtps::Config& obj)
{
    Node node;
    node["DiscoveryType"] = obj.discoveryType;
    node["UnicastLocators"] = obj.unicastLocators;
    node["ConfigFileName"] = obj.configFileName;
    auto optional_field = [&node](auto value, auto name) {
        if (value != -1)
            node[name] = value;
    };
    optional_field(obj.sendSocketBufferSize, "SendSocketBufferSize");
    optional_field(obj.listenSocketBufferSize, "ListenSocketBufferSize");
    optional_field(obj.historyDepth, "HistoryDepth");
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FastRtps::Config& obj)
{

    optional_decode(obj.discoveryType, node, "DiscoveryType");
    optional_decode(obj.configFileName, node, "ConfigFileName");
    optional_decode(obj.unicastLocators,node,"UnicastLocators");

    switch (obj.discoveryType)
    {
    case FastRtps::DiscoveryType::Local:
        if (!obj.unicastLocators.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: UnicastLocators must not be specified when using DiscoveryType Local");
        }

        if (!obj.configFileName.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: Using a FastRTPS configuration file requires DiscoverType ConfigFile");
        }

        break;

    case FastRtps::DiscoveryType::Multicast:
        if (!obj.unicastLocators.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: UnicastLocators must not be specified when using DiscoveryType Multicast");
        }

        if (!obj.configFileName.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: Using a FastRTPS configuration file requires DiscoverType ConfigFile");
        }

        break;

    case FastRtps::DiscoveryType::Unicast:
        if (obj.unicastLocators.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: DiscoveryType Unicast requires UnicastLocators being specified");
        }

        if (!obj.configFileName.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: Using a FastRTPS configuration file requires DiscoverType ConfigFile");
        }

        break;

    case FastRtps::DiscoveryType::ConfigFile:
        if (!obj.unicastLocators.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: UnicastLocators must not be specified when using DiscoveryType Multicast");
        }

        if (obj.configFileName.empty())
        {
            throw BadVibConversion(node, "FastRtps::Config: DiscoveryType ConfigFile requires ConfigFileName being specified");
        }

        break;

    default:
        throw BadVibConversion(node, "Invalid FastRTPS discovery type: " + to_string(obj.discoveryType));
    }

    optional_decode(obj.sendSocketBufferSize, node, "SendSocketBufferSize");
    optional_decode(obj.listenSocketBufferSize, node, "ListenSocketBufferSize");
    optional_decode(obj.historyDepth, node, "HistoryDepth");
    if (node["HistoryDepth"])
    {
        if (obj.historyDepth <= 0)
        {
            throw BadVibConversion(node, "FastRtps::Config: FastRTPS HistoryDepth must be above 0");
        }
    }
    return true;
}

template<>
Node VibConversion::encode(const VAsio::RegistryConfig& obj)
{
    static const VAsio::RegistryConfig defaultObj;
    Node node;
    non_default_encode(obj.hostname, node, "Hostname", defaultObj.hostname);
    non_default_encode(obj.port, node, "Port", defaultObj.port);
    non_default_encode(obj.logger, node, "Logger", defaultObj.logger);
    non_default_encode(obj.connectAttempts, node, "ConnectAttempts", defaultObj.connectAttempts);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, VAsio::RegistryConfig& obj)
{
    optional_decode(obj.hostname, node, "Hostname");
    optional_decode(obj.port, node, "Port");
    optional_decode(obj.logger, node, "Logger");
    optional_decode(obj.connectAttempts, node, "ConnectAttempts");

    if (obj.connectAttempts < 1)
    {
        obj.connectAttempts = 1;
    }
    return true;
}

template<>
Node VibConversion::encode(const VAsio::Config& obj)
{
    Node node;
    static const VAsio::Config defaultObj;

    non_default_encode(obj.registry, node, "Registry", defaultObj.registry);
    non_default_encode(obj.tcpNoDelay, node, "TcpNoDelay", defaultObj.tcpNoDelay);
    non_default_encode(obj.tcpQuickAck, node, "TcpQuickAck", defaultObj.tcpQuickAck);
    non_default_encode(obj.tcpReceiveBufferSize, node, "TcpReceiveBufferSize", defaultObj.tcpReceiveBufferSize);
    non_default_encode(obj.tcpSendBufferSize, node, "TcpSendBufferSize", defaultObj.tcpSendBufferSize);
    non_default_encode(obj.enableDomainSockets, node, "EnableDomainSockets", defaultObj.enableDomainSockets);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, VAsio::Config& obj)
{
    optional_decode(obj.registry, node, "Registry");
    optional_decode(obj.tcpNoDelay, node, "TcpNoDelay");
    optional_decode(obj.tcpQuickAck, node, "TcpQuickAck");
    optional_decode(obj.tcpReceiveBufferSize, node, "TcpReceiveBufferSize");
    optional_decode(obj.tcpSendBufferSize, node, "TcpSendBufferSize");
    optional_decode(obj.enableDomainSockets, node, "EnableDomainSockets");
    return true;
}

template<>
Node VibConversion::encode(const Middleware& obj)
{
    Node node;
    try
    {
        node = to_string(obj);
    }
    catch (const ib::type_conversion_error&)
    {
        node = "UNKNOWN_MIDDLEWARE";
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Middleware& obj)
{
    try
    {
        obj = from_string<Middleware>(parse_as<std::string>(node));
    }
    catch (const type_conversion_error&)
    {
        throw BadVibConversion(node, "Unknown Middleware: " + parse_as<std::string>(node));
    }
    return true;
}

template<>
Node VibConversion::encode(const MiddlewareConfig& obj)
{
    const static MiddlewareConfig defaultObj{};
    Node node;
    non_default_encode(obj.activeMiddleware, node, "ActiveMiddleware", defaultObj.activeMiddleware);
    non_default_encode(obj.fastRtps, node, "FastRTPS", defaultObj.fastRtps);
    non_default_encode(obj.vasio, node, "VAsio", defaultObj.vasio);
    return node;
}

template<>
bool VibConversion::decode(const Node& node, MiddlewareConfig& obj)
{
    optional_decode(obj.activeMiddleware, node, "ActiveMiddleware");
    optional_decode(obj.fastRtps, node, "FastRTPS");
    optional_decode(obj.vasio, node, "VAsio");
    return true;
}

template<>
Node VibConversion::encode(const ExtensionConfig& obj)
{
    static const ExtensionConfig defaultObj;
    Node node;
    non_default_encode(obj.searchPathHints, node, "SearchPathHints", defaultObj.searchPathHints);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, ExtensionConfig& obj)
{
    optional_decode(obj.searchPathHints, node, "SearchPathHints");
    return true;
}

template<>
Node VibConversion::encode(const Config& obj)
{
    static const Config defaultObj;
    Node node;
    non_default_encode(obj.version, node, "ConfigVersion", defaultObj.version);
    node["SchemaVersion"] = obj.schemaVersion;
    node["ConfigName"] = obj.name;
    node["Description"] = obj.description;
    node["SimulationSetup"] = obj.simulationSetup;
    non_default_encode(obj.middlewareConfig, node, "MiddlewareConfig", defaultObj.middlewareConfig);
    non_default_encode(obj.extensionConfig, node, "ExtensionConfig", defaultObj.extensionConfig);
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Config& obj)
{
    optional_decode(obj.version, node, "ConfigVersion");
    optional_decode(obj.name, node, "ConfigName");
    optional_decode(obj.description, node, "Description");
    optional_decode(obj.simulationSetup, node, "SimulationSetup");
    optional_decode(obj.middlewareConfig, node, "MiddlewareConfig");
    optional_decode(obj.schemaVersion, node, "SchemaVersion");
    optional_decode(obj.extensionConfig, node, "ExtensionConfig");
    return true;
}

template<>
Node VibConversion::encode(const TraceSink& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["Type"] = obj.type;
    node["OutputPath"] = obj.outputPath;
    //only serialize if disabled
    if (!obj.enabled)
    {
        node["Enabled"] = obj.enabled;
    }

    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSink& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.type = parse_as<decltype(obj.type)>(node["Type"]);
    obj.outputPath = parse_as<decltype(obj.outputPath)>(node["OutputPath"]);
    if (node["Enabled"])
    {
        obj.enabled = parse_as<decltype(obj.enabled)>(node["Enabled"]);
    }
    return true;
}

template<>
Node VibConversion::encode(const TraceSink::Type& obj)
{
    Node node;
    switch (obj)
    {
    case TraceSink::Type::Undefined:
        node =  "Undefined";
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
        throw Misconfiguration{"Unknown TraceSink Type"};
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSink::Type& obj)
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
        throw BadVibConversion(node, "Unknown TraceSink::Type: " + str + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const TraceSource& obj)
{
    Node node;
    node["Name"] = obj.name;
    node["Type"] = obj.type;
    node["InputPath"] = obj.inputPath;
    //only serialize if disabled
    if (!obj.enabled)
    {
        node["Enabled"] = obj.enabled;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSource& obj)
{
    obj.name = parse_as<std::string>(node["Name"]);
    obj.type = parse_as<decltype(obj.type)>(node["Type"]);
    obj.inputPath = parse_as<decltype(obj.inputPath)>(node["InputPath"]);
    if (node["Enabled"])
    {
        obj.enabled = parse_as<decltype(obj.enabled)>(node["Enabled"]);
    }
    return true;
}

template<>
Node VibConversion::encode(const TraceSource::Type& obj)
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
        throw Misconfiguration{"Unknown TraceSource Type"};
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, TraceSource::Type& obj)
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
        throw BadVibConversion(node, "Unknown TraceSource::Type: " + str + ".");
    }
    return true;
}

template<>
Node VibConversion::encode(const Replay& obj)
{
    static const Replay defaultObj;
    Node node;
    node["UseTraceSource"] = obj.useTraceSource;
    non_default_encode(obj.direction, node, "Direction", defaultObj.direction);
    non_default_encode(obj.mdfChannel, node, "MdfChannel", defaultObj.mdfChannel);

    return node;
}
template<>
bool VibConversion::decode(const Node& node, Replay& obj)
{
    obj.useTraceSource = parse_as<decltype(obj.useTraceSource)>(node["UseTraceSource"]);
    optional_decode(obj.direction, node, "Direction");
    optional_decode(obj.mdfChannel, node, "MdfChannel");
    return true;
}

template<>
Node VibConversion::encode(const Replay::Direction& obj)
{
    Node node;
    switch (obj)
    {
    case Replay::Direction::Send:
        node =  "Send";
        break;
    case Replay::Direction::Receive:
        node =   "Receive";
        break;
    case Replay::Direction::Both:
        node =  "Both";
        break;
    case Replay::Direction::Undefined:
        node =  "Undefined";
        break;
    }
    return node;
}
template<>
bool VibConversion::decode(const Node& node, Replay::Direction& obj)
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
        throw BadVibConversion(node, "Unknown Replay::Direction: " + str + ".");
    }
    return true;
}
} // namespace YAML
