#include "JsonConfig.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "ib/cfg/string_utils.hpp"
#include "ib/exception.hpp"

namespace ib {
namespace cfg {

// Put helper functions in anonymous namespace
namespace {

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

template <typename ConfigT>
auto optional_from_json(ConfigT& cfg, const json11::Json& json, const std::string& fieldName)
{
    if (json.object_items().count(fieldName))
        cfg = from_json<ConfigT>(json[fieldName]);
}


} // namespace anonymous


auto to_json(uint16_t value) -> json11::Json
{
    return json11::Json{value};
}

template<>
auto from_json<uint16_t>(const json11::Json& json) -> uint16_t
{
    return static_cast<uint16_t>(json.int_value());
}

auto to_json(int32_t value) -> json11::Json
{
    return json11::Json{ value };
}

template<>
auto from_json<int32_t>(const json11::Json& json) -> int32_t
{
    return json.int_value();
}

auto to_json(const std::string& value) -> json11::Json
{
    return json11::Json{value};
}

template<>
auto from_json<std::string>(const json11::Json& json) -> std::string
{
    return json.string_value();
}


auto to_json(const Version& version) -> json11::Json
{
    std::stringstream versionOut;
    versionOut << version;
    return json11::Json{versionOut.str()};
}

template <>
auto from_json<Version>(const json11::Json& json) -> Version
{
    std::stringstream in(json.string_value());

    Version version;
    in >> version;
    return version;
}

auto to_json(const CanController& controller) -> json11::Json
{
    return json11::Json(controller.name);
}

template <>
auto from_json<CanController>(const json11::Json& json) -> CanController
{
    CanController controller;
    controller.name = json.string_value();
    return controller;
}

auto to_json(const LinController& controller) -> json11::Json
{
    return json11::Json(controller.name);
}

template <>
auto from_json<LinController>(const json11::Json& json) -> LinController
{
    LinController controller;
    controller.name = json.string_value();
    return controller;
}

auto macaddress_to_json(const std::array<uint8_t, 6>& macAddress) -> json11::Json
{
    std::stringstream macOut;

    to_ostream(macOut, macAddress);

    return json11::Json(macOut.str());
}

auto macaddress_from_json(const json11::Json& json) -> std::array<uint8_t, 6>
{
    std::array<uint8_t, 6> macAddress;

    std::stringstream macIn(json.string_value());
    from_istream(macIn, macAddress);

    return macAddress;
}


auto to_json(const EthernetController& controller) -> json11::Json
{
    return json11::Json::object {
        { "Name", controller.name },
        // endopintId is not serialized
        { "MacAddr",  macaddress_to_json(controller.macAddress) }
    };
}

template <>
auto from_json<EthernetController>(const json11::Json& json) -> EthernetController
{
    EthernetController controller;
    controller.name = json["Name"].string_value();
    controller.macAddress = macaddress_from_json(json["MacAddr"]);
    return controller;
}

auto to_json(const sim::fr::ClusterParameters& clusterParameters) -> json11::Json
{
    return json11::Json::object{
        { "gColdstartAttempts", clusterParameters.gColdstartAttempts },
        { "gCycleCountMax", clusterParameters.gCycleCountMax },
        { "gdActionPointOffset", clusterParameters.gdActionPointOffset },
        { "gdDynamicSlotIdlePhase", clusterParameters.gdDynamicSlotIdlePhase },
        { "gdMiniSlot", clusterParameters.gdMiniSlot },
        { "gdMiniSlotActionPointOffset", clusterParameters.gdMiniSlotActionPointOffset },
        { "gdStaticSlot", clusterParameters.gdStaticSlot },
        { "gdSymbolWindow", clusterParameters.gdSymbolWindow },
        { "gdSymbolWindowActionPointOffset", clusterParameters.gdSymbolWindowActionPointOffset },
        { "gdTSSTransmitter", clusterParameters.gdTSSTransmitter },
        { "gdWakeupTxActive", clusterParameters.gdWakeupTxActive },
        { "gdWakeupTxIdle", clusterParameters.gdWakeupTxIdle },
        { "gListenNoise", clusterParameters.gListenNoise },
        { "gMacroPerCycle", clusterParameters.gMacroPerCycle },
        { "gMaxWithoutClockCorrectionFatal", clusterParameters.gMaxWithoutClockCorrectionFatal },
        { "gMaxWithoutClockCorrectionPassive", clusterParameters.gMaxWithoutClockCorrectionPassive },
        { "gNumberOfMiniSlots", clusterParameters.gNumberOfMiniSlots },
        { "gNumberOfStaticSlots", clusterParameters.gNumberOfStaticSlots },
        { "gPayloadLengthStatic", clusterParameters.gPayloadLengthStatic },
        { "gSyncFrameIDCountMax", clusterParameters.gSyncFrameIDCountMax }
    };
}

template <>
auto from_json<sim::fr::ClusterParameters>(const json11::Json& json) -> sim::fr::ClusterParameters
{
    sim::fr::ClusterParameters clusterParameters;
    clusterParameters.gColdstartAttempts = static_cast<uint8_t>(json["gColdstartAttempts"].int_value());
    clusterParameters.gCycleCountMax = static_cast<uint8_t>(json["gCycleCountMax"].int_value());
    clusterParameters.gdActionPointOffset = static_cast<uint16_t>(json["gdActionPointOffset"].int_value());
    clusterParameters.gdDynamicSlotIdlePhase = static_cast<uint16_t>(json["gdDynamicSlotIdlePhase"].int_value());
    clusterParameters.gdMiniSlot = static_cast<uint16_t>(json["gdMiniSlot"].int_value());
    clusterParameters.gdMiniSlotActionPointOffset = static_cast<uint16_t>(json["gdMiniSlotActionPointOffset"].int_value());
    clusterParameters.gdStaticSlot = static_cast<uint16_t>(json["gdStaticSlot"].int_value());
    clusterParameters.gdSymbolWindow = static_cast<uint16_t>(json["gdSymbolWindow"].int_value());
    clusterParameters.gdSymbolWindowActionPointOffset = static_cast<uint16_t>(json["gdSymbolWindowActionPointOffset"].int_value());
    clusterParameters.gdTSSTransmitter = static_cast<uint16_t>(json["gdTSSTransmitter"].int_value());
    clusterParameters.gdWakeupTxActive = static_cast<uint16_t>(json["gdWakeupTxActive"].int_value());
    clusterParameters.gdWakeupTxIdle = static_cast<uint16_t>(json["gdWakeupTxIdle"].int_value());
    clusterParameters.gListenNoise = static_cast<uint8_t>(json["gListenNoise"].int_value());
    clusterParameters.gMacroPerCycle = static_cast<uint16_t>(json["gMacroPerCycle"].int_value());
    clusterParameters.gMaxWithoutClockCorrectionFatal = static_cast<uint8_t>(json["gMaxWithoutClockCorrectionFatal"].int_value());
    clusterParameters.gMaxWithoutClockCorrectionPassive = static_cast<uint8_t>(json["gMaxWithoutClockCorrectionPassive"].int_value());
    clusterParameters.gNumberOfMiniSlots = static_cast<uint16_t>(json["gNumberOfMiniSlots"].int_value());
    clusterParameters.gNumberOfStaticSlots = static_cast<uint16_t>(json["gNumberOfStaticSlots"].int_value());
    clusterParameters.gPayloadLengthStatic = static_cast<uint16_t>(json["gPayloadLengthStatic"].int_value());
    clusterParameters.gSyncFrameIDCountMax = static_cast<uint8_t>(json["gSyncFrameIDCountMax"].int_value());
    return clusterParameters;
}

auto to_json(const sim::fr::NodeParameters& nodeParameters) -> json11::Json
{
    return json11::Json::object{
        { "pAllowHaltDueToClock", nodeParameters.pAllowHaltDueToClock },
        { "pAllowPassiveToActive", nodeParameters.pAllowPassiveToActive },
        { "pChannels", static_cast<uint8_t>(nodeParameters.pChannels) },
        { "pClusterDriftDamping", nodeParameters.pClusterDriftDamping },
        { "pdAcceptedStartupRange", nodeParameters.pdAcceptedStartupRange },
        { "pdListenTimeout", nodeParameters.pdListenTimeout },
        { "pKeySlotId", nodeParameters.pKeySlotId },
        { "pKeySlotOnlyEnabled", nodeParameters.pKeySlotOnlyEnabled },
        { "pKeySlotUsedForStartup", nodeParameters.pKeySlotUsedForStartup },
        { "pKeySlotUsedForSync", nodeParameters.pKeySlotUsedForSync },
        { "pLatestTx", nodeParameters.pLatestTx },
        { "pMacroInitialOffsetA", nodeParameters.pMacroInitialOffsetA },
        { "pMacroInitialOffsetB", nodeParameters.pMacroInitialOffsetB },
        { "pMicroInitialOffsetA", nodeParameters.pMicroInitialOffsetA },
        { "pMicroInitialOffsetB", nodeParameters.pMicroInitialOffsetB },
        { "pMicroPerCycle", nodeParameters.pMicroPerCycle },
        { "pOffsetCorrectionOut", nodeParameters.pOffsetCorrectionOut },
        { "pOffsetCorrectionStart", nodeParameters.pOffsetCorrectionStart },
        { "pRateCorrectionOut", nodeParameters.pRateCorrectionOut },
        { "pWakeupChannel", static_cast<uint8_t>(nodeParameters.pWakeupChannel) },
        { "pWakeupPattern", nodeParameters.pWakeupPattern },
        { "pdMicrotick", static_cast<uint8_t>(nodeParameters.pdMicrotick) },
        { "pSamplesPerMicrotick", nodeParameters.pSamplesPerMicrotick }
    };
}

template <>
auto from_json<sim::fr::NodeParameters>(const json11::Json& json) -> sim::fr::NodeParameters
{
    sim::fr::NodeParameters nodeParameters;
    nodeParameters.pAllowHaltDueToClock = static_cast<uint8_t>(json["pAllowHaltDueToClock"].int_value());
    nodeParameters.pAllowPassiveToActive = static_cast<uint8_t>(json["pAllowPassiveToActive"].int_value());
    nodeParameters.pChannels = static_cast<sim::fr::Channel>(json["pChannels"].int_value());
    nodeParameters.pClusterDriftDamping = static_cast<uint8_t>(json["pClusterDriftDamping"].int_value());
    nodeParameters.pdAcceptedStartupRange = static_cast<sim::fr::FrMickroTick>(json["pdAcceptedStartupRange"].int_value());
    nodeParameters.pdListenTimeout = static_cast<sim::fr::FrMickroTick>(json["pdListenTimeout"].int_value());
    nodeParameters.pKeySlotId = static_cast<uint16_t>(json["pKeySlotId"].int_value());
    nodeParameters.pKeySlotOnlyEnabled = static_cast<uint8_t>(json["pKeySlotOnlyEnabled"].int_value());
    nodeParameters.pKeySlotUsedForStartup = static_cast<uint8_t>(json["pKeySlotUsedForStartup"].int_value());
    nodeParameters.pKeySlotUsedForSync = static_cast<uint8_t>(json["pKeySlotUsedForSync"].int_value());
    nodeParameters.pLatestTx = static_cast<uint16_t>(json["pLatestTx"].int_value());
    nodeParameters.pMacroInitialOffsetA = static_cast<uint8_t>(json["pMacroInitialOffsetA"].int_value());
    nodeParameters.pMacroInitialOffsetB = static_cast<uint8_t>(json["pMacroInitialOffsetB"].int_value());
    nodeParameters.pMicroInitialOffsetA = static_cast<sim::fr::FrMickroTick>(json["pMicroInitialOffsetA"].int_value());
    nodeParameters.pMicroInitialOffsetB = static_cast<sim::fr::FrMickroTick>(json["pMicroInitialOffsetB"].int_value());
    nodeParameters.pMicroPerCycle = static_cast<sim::fr::FrMickroTick>(json["pMicroPerCycle"].int_value());
    nodeParameters.pOffsetCorrectionOut = static_cast<sim::fr::FrMickroTick>(json["pOffsetCorrectionOut"].int_value());
    nodeParameters.pOffsetCorrectionStart = static_cast<uint16_t>(json["pOffsetCorrectionStart"].int_value());
    nodeParameters.pRateCorrectionOut = static_cast<sim::fr::FrMickroTick>(json["pRateCorrectionOut"].int_value());
    nodeParameters.pWakeupChannel = static_cast<sim::fr::Channel>(json["pWakeupChannel"].int_value());
    nodeParameters.pWakeupPattern = static_cast<uint8_t>(json["pWakeupPattern"].int_value());
    nodeParameters.pdMicrotick = static_cast<sim::fr::ClockPeriod>(json["pdMicrotick"].int_value());
    nodeParameters.pSamplesPerMicrotick = static_cast<uint8_t>(json["pSamplesPerMicrotick"].int_value());
    return nodeParameters;
}

auto to_json(const FlexrayController& controller) -> json11::Json
{
    return json11::Json::object{
        { "Name", controller.name },
        { "ClusterParameters",  to_json(controller.clusterParameters) },
        { "NodeParameters", to_json(controller.nodeParameters) }
    };
}

template <>
auto from_json<FlexrayController>(const json11::Json& json) -> FlexrayController
{
    FlexrayController controller;

    // backward compatibility to old json config files with only name of controller
    if (json.is_string())
    {
        controller.name = json.string_value();
        return controller;
    }

    controller.name = json["Name"].string_value();
    controller.clusterParameters = from_json<sim::fr::ClusterParameters>(json["ClusterParameters"]);
    controller.nodeParameters = from_json<sim::fr::NodeParameters>(json["NodeParameters"]);
    return controller;
}

auto to_json(const DigitalIoPort& port) -> json11::Json
{
    return json11::Json::object{
        { port.name, port.initvalue }
    };
}

template <>
auto from_json(const json11::Json& json) -> DigitalIoPort
{
    DigitalIoPort port;
    port.direction = PortDirection::Out;

    if (json.object_items().size() == 0)
        throw Misconfiguration{"Digital IO has no properties!"};

    auto&& jsonPort = *json.object_items().begin();

    port.name = jsonPort.first;
    port.initvalue = jsonPort.second.bool_value();

    return port;
}

auto to_json(const AnalogIoPort& port) -> json11::Json
{
    return json11::Json::object {
        { port.name,
            json11::Json::object {
                {"value", port.initvalue},
                {"unit", port.unit}
            }
        }
    };
}

template <>
auto from_json(const json11::Json& json) -> AnalogIoPort
{
    AnalogIoPort port;
    port.direction = PortDirection::Out;

    if (json.object_items().size() == 0)
        throw Misconfiguration{"Analog IO has no properties!"};

    auto&& jsonPort = *json.object_items().begin();

    port.name = jsonPort.first;
    port.initvalue = jsonPort.second["value"].number_value();
    port.unit = jsonPort.second["unit"].string_value();

    return port;
}

auto to_json(const PwmPort& port) -> json11::Json
{
    return json11::Json::object {
        { port.name, json11::Json::object {
                { "freq", json11::Json::object {{ "value", port.initvalue.frequency }, {"unit", port.unit }}},
                { "duty", port.initvalue.dutyCycle }
        }}
    };
}

template <>
auto from_json(const json11::Json& json) -> PwmPort
{
    PwmPort port;
    port.direction = PortDirection::Out;

    if (json.object_items().size() == 0)
        throw Misconfiguration{"PWM IO has no properties!"};

    auto&& jsonPort = *json.object_items().begin();

    port.name = jsonPort.first;
    port.initvalue = {jsonPort.second["freq"]["value"].number_value(), jsonPort.second["duty"].number_value()};
    port.unit = jsonPort.second["freq"]["unit"].string_value();

    return port;
}

auto to_json(const PatternPort& port) -> json11::Json
{
    return json11::Json::object{
        {port.name, hex_encode(port.initvalue)}
    };
}

template <>
auto from_json(const json11::Json& json) -> PatternPort
{
    PatternPort port;
    port.direction = PortDirection::Out;

    if (json.object_items().size() == 0)
        throw Misconfiguration{"Pattern IO has no properties!"};

    auto&& jsonPort = *json.object_items().begin();

    port.name = jsonPort.first;
    port.initvalue = hex_decode(jsonPort.second.string_value());

    return port;
}

auto to_json(GenericPort::ProtocolType protocolType) -> json11::Json
{
    switch (protocolType)
    {
    case GenericPort::ProtocolType::ROS:
        return "ROS";
    case GenericPort::ProtocolType::SOMEIP:
        return "SOME/IP";
    default:
        return "";
    }
}

template <>
auto from_json(const json11::Json& json) -> GenericPort::ProtocolType
{
    auto&& protocolName = json.string_value();
    if (protocolName == "ROS")
        return GenericPort::ProtocolType::ROS;
    if (protocolName == "SOME/IP")
        return GenericPort::ProtocolType::SOMEIP;
    if (protocolName == "Undefined")
        return GenericPort::ProtocolType::Undefined;
    if (protocolName == "")
        return GenericPort::ProtocolType::Undefined;

    throw Misconfiguration{"Invalid ProtocolType"};
}

auto to_json(const GenericPort& port) -> json11::Json
{
    return json11::Json::object{
        {"Name", port.name},
        {"Protocol", to_json(port.protocolType)},
        {"DefinitionUri", port.definitionUri}
    };
}

template <>
auto from_json(const json11::Json& json) -> GenericPort
{
    GenericPort port;

    port.name = json["Name"].string_value();

    // Optional "Protocol" property
    optional_from_json(port.protocolType, json, "Protocol");

    // Optional "Protocol" property
    optional_from_json(port.definitionUri, json, "DefinitionUri");

    return port;
}

auto to_json(SyncType syncType) -> json11::Json
{
    try
    {
        return to_string(syncType);
    }
    catch (const ib::type_conversion_error&)
    {
        return "UNKNOWN_SYNC_TYPE";
    }
}

template <>
auto from_json<SyncType>(const json11::Json& json) -> SyncType
{
    auto&& syncType = json.string_value();

    if (syncType == "DiscreteEvent")
        return SyncType::DiscreteEvent;
    if (syncType == "TimeQuantum")
        return SyncType::TimeQuantum;
    if (syncType == "DiscreteTime")
        return SyncType::DiscreteTime;
    if (syncType == "DiscreteTimePassive")
        return SyncType::DiscreteTimePassive;
    if (syncType == "Unsynchronized")
        return SyncType::Unsynchronized;

    throw Misconfiguration{"Invalid Participant SyncType"};
}

auto to_json(const Participant& participant) -> json11::Json
{
    auto makePortList =
        [](auto&& portVector, PortDirection direction)
        {
            std::vector<std::string> result;
            result.reserve(portVector.size());
            for (auto&& port : portVector)
                if (port.direction == direction)
                    result.push_back(port.name);
            return result;
        };

    auto subscribers_to_json =
        [](auto&& subscribers)
        {
            std::vector<std::string> result{subscribers.size()};
            std::transform(subscribers.begin(), subscribers.end(), result.begin(), [](auto&& sub) { return sub.name; });
            return result;
        };

    return json11::Json::object{
        {"Name", participant.name},
        {"CanControllers", to_json(participant.canControllers)},
        {"LinControllers", to_json(participant.linControllers)},
        {"EthernetControllers", to_json(participant.ethernetControllers)},
        {"FlexRayControllers", to_json(participant.flexrayControllers)},
        {"NetworkSimulators", participant.networkSimulators},
        {"Analog-In", makePortList(participant.analogIoPorts, PortDirection::In)},
        {"Digital-In", makePortList(participant.analogIoPorts, PortDirection::In)},
        {"Pwm-In", makePortList(participant.analogIoPorts, PortDirection::In)},
        {"Pattern-In", makePortList(participant.analogIoPorts, PortDirection::In)},
        {"Analog-Out", makePortList(participant.analogIoPorts, PortDirection::Out)},
        {"Digital-Out", makePortList(participant.analogIoPorts, PortDirection::Out)},
        {"Pwm-Out", makePortList(participant.analogIoPorts, PortDirection::Out)},
        {"Pattern-Out", makePortList(participant.analogIoPorts, PortDirection::Out)},
        {"GenericPublishers", to_json(participant.genericPublishers)},
        {"GenericSubscribers", subscribers_to_json(participant.genericSubscribers)},
        {"SyncType", to_json(participant.syncType)},
        {"IsSyncMaster", participant.isSyncMaster}
    };
}

template <>
auto from_json<Participant>(const json11::Json& json) -> Participant
{
    Participant participant;

    auto subscribers_from_json =
        [](auto&& jsonSubscribers)
        {
            std::vector<GenericPort> result;
            result.reserve(jsonSubscribers.size());
            for (auto&& subscriber : jsonSubscribers)
            {
                GenericPort port;
                port.name = subscriber.string_value();
                result.emplace_back(std::move(port));

            }
            return result;
        };

    // assing Input Ports
    // Input Ports are specified simply by name. So we have to infer the corresponding *IoPort objects.
    auto inports_from_json =
        [json](auto&& portList, auto&& propertyName)
        {
            for (auto&& inPort : json[propertyName].array_items())
            {
                using PortType = typename std::decay_t<decltype(portList)>::value_type;
                PortType port;
                port.name = inPort.string_value();
                port.direction = PortDirection::In;
                portList.push_back(port);
            }
        };


    participant.name = json["Name"].string_value();

    participant.canControllers = from_json<std::vector<CanController>>(json["CanControllers"].array_items());
    participant.linControllers = from_json<std::vector<LinController>>(json["LinControllers"].array_items());
    participant.ethernetControllers = from_json<std::vector<EthernetController>>(json["EthernetControllers"].array_items());
    participant.flexrayControllers = from_json<std::vector<FlexrayController>>(json["FlexRayControllers"].array_items());
    participant.networkSimulators = from_json<std::vector<std::string>>(json["NetworkSimulators"].array_items());

    // assign Output Ports
    participant.digitalIoPorts = from_json<std::vector<DigitalIoPort>>(json["Digital-Out"].array_items());
    participant.analogIoPorts = from_json<std::vector<AnalogIoPort>>(json["Analog-Out"].array_items());
    participant.pwmPorts = from_json<std::vector<PwmPort>>(json["Pwm-Out"].array_items());
    participant.patternPorts = from_json<std::vector<PatternPort>>(json["Pattern-Out"].array_items());

    inports_from_json(participant.digitalIoPorts, "Digital-In");
    inports_from_json(participant.analogIoPorts, "Analog-In");
    inports_from_json(participant.pwmPorts, "Pwm-In");
    inports_from_json(participant.patternPorts, "Pattern-In");

    participant.genericPublishers = from_json<std::vector<GenericPort>>(json["GenericPublishers"].array_items());
    participant.genericSubscribers = subscribers_from_json(json["GenericSubscribers"].array_items());

    optional_from_json(participant.syncType, json, "SyncType");

    participant.isSyncMaster = json["IsSyncMaster"].bool_value();

    return participant;
}

auto to_json(const Switch::Port& switchPort) -> json11::Json
{
    return json11::Json::object{
        { "Name", switchPort.name },
        { "VlanIds", switchPort.vlanIds }
    };
}

template <>
auto from_json<Switch::Port>(const json11::Json& json) -> Switch::Port
{
    Switch::Port port;

    port.name = json["Name"].string_value();
    port.vlanIds = from_json<std::vector<uint16_t>>(json["VlanIds"].array_items());
    return port;
}

auto to_json(const Switch& switch_) -> json11::Json
{
    return json11::Json::object{
        { "Name", switch_.name },
        { "Description", switch_.description },
        { "Ports", to_json(switch_.ports) }
    };
}

template <>
auto from_json<Switch>(const json11::Json& json) -> Switch
{
    Switch switch_;

    switch_.name = json["Name"].string_value();
    switch_.description = json["Description"].string_value();
    switch_.ports = from_json<decltype(switch_.ports)>(json["Ports"].array_items());

    return switch_;
}

auto to_json(const Link& link) -> json11::Json
{
    return json11::Json::object{
        { "Name", link.name },
        { "Endpoints", link.endpoints }
    };
}

template <>
auto from_json<Link>(const json11::Json& json) -> Link
{
    Link link;
    link.name = json["Name"].string_value();
    link.endpoints = from_json<std::vector<std::string>>(json["Endpoints"].array_items());
    return link;
}

auto to_json(const NetworkSimulator& networkSimulator) -> json11::Json
{
    return json11::Json::object{
        { "Name", networkSimulator.name },
        { "SimulatedLinks", networkSimulator.simulatedLinks },
        { "SimulatedSwitches", networkSimulator.simulatedSwitches }
    };
}

template <>
auto from_json<NetworkSimulator>(const json11::Json& json) -> NetworkSimulator
{
    NetworkSimulator simulator;

    simulator.name = json["Name"].string_value();
    simulator.simulatedLinks = from_json<std::vector<std::string>>(json["SimulatedLinks"].array_items());
    simulator.simulatedSwitches = from_json<std::vector<std::string>>(json["SimulatedSwitches"].array_items());
    return simulator;
}

auto to_json(TimeSync::SyncPolicy syncPolicy) -> json11::Json
{
    try
    {
        return to_string(syncPolicy);
    }
    catch (const ib::type_conversion_error&)
    {
        return "UNKNOWN_SYNC_POLICY";
    }
}

template <>
auto from_json<TimeSync::SyncPolicy>(const json11::Json& json) -> TimeSync::SyncPolicy
{
    auto&& syncType = json.string_value();

    if (syncType == "Loose")
        return TimeSync::SyncPolicy::Loose;
    if (syncType == "Strict")
        return TimeSync::SyncPolicy::Strict;

    throw Misconfiguration{"Invalid Participant SyncPolicy"};
}

auto to_json(const TimeSync& timeSync) -> json11::Json
{
    return json11::Json::object{
        { "SyncPolicy", to_json(timeSync.syncPolicy) },
        { "TickPeriodNs", static_cast<int>(timeSync.tickPeriod.count()) }
    };
}

template <>
auto from_json<TimeSync>(const json11::Json& json) -> TimeSync
{
    TimeSync timeSync;

    optional_from_json(timeSync.syncPolicy, json, "SyncPolicy");
    timeSync.tickPeriod = std::chrono::nanoseconds{json["TickPeriodNs"].int_value()};

    return timeSync;
}

auto to_json(const SimulationSetup& simulationSetup) -> json11::Json
{
    return json11::Json::object
    {
        {"Participants", to_json(simulationSetup.participants)},
        {"Switches", to_json(simulationSetup.switches)},
        {"Links", to_json(simulationSetup.links)},
        {"NetworkSimulators", to_json(simulationSetup.networkSimulators)},
        {"TimeSync", to_json(simulationSetup.timeSync)}
    };
}

template <>
auto from_json<SimulationSetup>(const json11::Json& json) -> SimulationSetup
{
    SimulationSetup simulationSetup;
    simulationSetup.participants = from_json<std::vector<Participant>>(json["Participants"].array_items());
    simulationSetup.switches = from_json<std::vector<Switch>>(json["Switches"].array_items());
    simulationSetup.links = from_json<std::vector<Link>>(json["Links"].array_items());
    simulationSetup.networkSimulators = from_json<std::vector<NetworkSimulator>>(json["NetworkSimulators"].array_items());
    simulationSetup.timeSync = from_json<TimeSync>(json["TimeSync"]);

    return simulationSetup;
}

auto to_json(FastRtps::DiscoveryType discoveryType) -> json11::Json
{
    try
    {
        return to_string(discoveryType);
    }
    catch (const ib::type_conversion_error&)
    {
        return "UNKNOWN_DISCOVERY_TYPE";
    }
}

template <>
auto from_json<FastRtps::DiscoveryType>(const json11::Json& json) -> FastRtps::DiscoveryType
{
    using namespace FastRtps;

    auto&& discoveryType = json.string_value();

    if (discoveryType == "Local")
        return DiscoveryType::Local;
    if (discoveryType == "Multicast")
        return DiscoveryType::Multicast;
    if (discoveryType == "Unicast")
        return DiscoveryType::Unicast;
    if (discoveryType == "ConfigFile")
        return DiscoveryType::ConfigFile;

    throw Misconfiguration{"Invalid FastRTPS DiscoveryType"};
}

auto to_json(const MiddlewareConfig& middlewareConfig) -> json11::Json
{
    return json11::Json::object
    {
        { "FastRTPS", to_json(middlewareConfig.fastRtps) }
    };
}

template <>
auto from_json<FastRtps::Config>(const json11::Json& json) -> FastRtps::Config
{
    FastRtps::Config fastRtps;

    optional_from_json(fastRtps.discoveryType, json, "DiscoveryType");
    fastRtps.configFileName = from_json<std::string>(json["ConfigFileName"]);

    for (auto&& kv : json["UnicastLocators"].object_items())
    {
        fastRtps.unicastLocators[kv.first] = kv.second.string_value();
    }


    switch (fastRtps.discoveryType)
    {
    case FastRtps::DiscoveryType::Local:
        if (!fastRtps.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Local"};

        if (!fastRtps.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::Multicast:
        if (!fastRtps.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Multicast"};

        if (!fastRtps.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::Unicast:
        if (fastRtps.unicastLocators.empty())
            throw Misconfiguration{"DiscoveryType Unicast requires UnicastLocators being specified"};

        if (!fastRtps.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::ConfigFile:
        if (!fastRtps.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Multicast"};

        if (fastRtps.configFileName.empty())
            throw Misconfiguration{"DiscoveryType ConfigFile requires ConfigFileName being specified"};

        break;

    default:
        throw Misconfiguration{"Invalid FastRTPS discovery type: " + to_string(fastRtps.discoveryType)};
    }

    fastRtps.sendSocketBufferSize = -1;
    optional_from_json(fastRtps.sendSocketBufferSize, json, "SendSocketBufferSize");
    fastRtps.listenSocketBufferSize = -1;
    optional_from_json(fastRtps.listenSocketBufferSize, json, "ListenSocketBufferSize");

    return fastRtps;
}

auto to_json(const FastRtps::Config& fastRtps) -> json11::Json
{
    return json11::Json::object
    {
        {"DiscoveryType", to_string(fastRtps.discoveryType)},
        {"UnicastLocators", fastRtps.unicastLocators},
        {"ConfigFileName", to_json(fastRtps.configFileName)},
        {"SendSocketBufferSize", to_json(fastRtps.sendSocketBufferSize)},
        {"ListenSocketBufferSize", to_json(fastRtps.listenSocketBufferSize)}
    };
}

template <>
auto from_json<MiddlewareConfig>(const json11::Json& json) -> MiddlewareConfig
{
    MiddlewareConfig middlewareConfig;
    middlewareConfig.fastRtps = from_json<FastRtps::Config>(json["FastRTPS"]);

    return middlewareConfig;
}

auto to_json(const Config& cfg) -> json11::Json
{
    return json11::Json::object
    {
        {"ConfigVersion", to_json(cfg.version)},
        {"ConfigName", cfg.name},
        {"Description", cfg.description},

        {"SimulationSetup", to_json(cfg.simulationSetup)},
        {"MiddlewareConfig", to_json(cfg.middlewareConfig)}
    };
}

template <>
auto from_json<Config>(const json11::Json& json) -> Config
{
    Config config;
    config.version     = from_json<Version>(json["ConfigVersion"]);
    config.name        = json["ConfigName"].string_value();
    config.description = json["Description"].string_value();

    config.simulationSetup = from_json<SimulationSetup>(json["SimulationSetup"]);
    config.middlewareConfig = from_json<MiddlewareConfig>(json["MiddlewareConfig"]);

    return config;
}


// ======================================================================
//  Generic Container Conversion
// ======================================================================
template<typename T>
auto to_json(const std::vector<T>& vector) -> json11::Json::array
{
    json11::Json::array array;
    array.reserve(vector.size());
    std::transform(vector.begin(), vector.end(),
                   std::back_inserter(array),
                   [](auto&& t) { return to_json(t); });
    return array;
}

template<typename T, class FUNC>
auto to_json_array(const std::vector<T>& vector, FUNC&& conversion) -> json11::Json::array
{
    json11::Json::array array;
    array.reserve(vector.size());
    std::transform(vector.begin(), vector.end(),
                   std::back_inserter(array),
                   std::forward<FUNC>(conversion));
    return array;
}

template<typename T, typename>
auto from_json(const json11::Json::array& array) -> T
{
    T vector;
    vector.reserve(array.size());
    std::transform(array.begin(), array.end(),
                   std::back_inserter(vector),
                   [](auto&& t) { return from_json<typename T::value_type>(t); });
    return vector;
}

template<typename T>
auto to_json(const std::map<std::string, T>& map) -> json11::Json::object
{
    json11::Json::object object;
    std::transform(map.begin(), map.end(),
                   std::inserter(object, object.end()),
                   [](auto&& kv) { return std::make_tuple(kv.first, to_json(kv.second)); });
    return object;
}


template<typename T, typename>
auto from_json(const json11::Json::object& object) -> T
{
    T map;
    std::transform(object.begin(), object.end(),
                   std::inserter(map, map.end()),
                   [](auto&& kv) { return std::make_tuple(kv.first, from_json<typename T::mapped_type>(kv.second)); });
    return map;
}

} // namespace cfg
} // namespace ib
