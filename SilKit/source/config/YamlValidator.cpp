// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "YamlValidator.hpp"

#include <stdexcept>
#include <set>
#include <deque>
#include <string>
#include <vector>
#include <sstream>


#include "Configuration.hpp"
#include "rapidyaml.hpp"

//#include "YamlConversion.hpp"
//#include "SilKitYamlHelper.hpp" // ParserContext
#include "YamlParser.hpp" // ParserContext

namespace {

inline auto to_string(ryml::csubstr stringView) -> std::string
{
    return {stringView.data(), stringView.size()};
}

// the rapidyaml visit_stacked does not accept reference types, so we implement it here

template <typename VisitorRef>
bool visit_stacked(ryml::ConstNodeRef& node, VisitorRef& visitor, ryml::id_type indentation_level = 0)
{
    ryml::id_type increment = 1;
    if (visitor(node, indentation_level))
    {
        return true;
    }
    if (node.has_children())
    {
        visitor.push(node, indentation_level);
        for (auto child : node.children())
        {
            if (visit_stacked(child, visitor, indentation_level + increment))
            {
                visitor.pop(node, indentation_level);
                return true;
            }
        }
        visitor.pop(node, indentation_level);
    }
    return false;
}

inline auto splitString(std::string s, std::string delimiter) -> std::vector<std::string>
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

const std::string schemaSeparator{"/"};

const std::set<std::string> schemaPaths_v1 = {
    "/$schema",
    "/CanControllers",
    "/CanControllers/Name",
    "/CanControllers/Network",
    "/CanControllers/Replay",
    "/CanControllers/Replay/Direction",
    "/CanControllers/Replay/MdfChannel",
    "/CanControllers/Replay/MdfChannel/ChannelName",
    "/CanControllers/Replay/MdfChannel/ChannelPath",
    "/CanControllers/Replay/MdfChannel/ChannelSource",
    "/CanControllers/Replay/MdfChannel/GroupName",
    "/CanControllers/Replay/MdfChannel/GroupPath",
    "/CanControllers/Replay/MdfChannel/GroupSource",
    "/CanControllers/Replay/UseTraceSource",
    "/CanControllers/UseTraceSinks",
    "/DataPublishers",
    "/DataPublishers/Labels",
    "/DataPublishers/Labels/Key",
    "/DataPublishers/Labels/Kind",
    "/DataPublishers/Labels/Value",
    "/DataPublishers/Name",
    "/DataPublishers/Replay",
    "/DataPublishers/Replay/Direction",
    "/DataPublishers/Replay/MdfChannel",
    "/DataPublishers/Replay/MdfChannel/ChannelName",
    "/DataPublishers/Replay/MdfChannel/ChannelPath",
    "/DataPublishers/Replay/MdfChannel/ChannelSource",
    "/DataPublishers/Replay/MdfChannel/GroupName",
    "/DataPublishers/Replay/MdfChannel/GroupPath",
    "/DataPublishers/Replay/MdfChannel/GroupSource",
    "/DataPublishers/Replay/UseTraceSource",
    "/DataPublishers/Topic",
    "/DataPublishers/History",
    "/DataPublishers/UseTraceSinks",
    "/DataSubscribers",
    "/DataSubscribers/Labels",
    "/DataSubscribers/Labels/Key",
    "/DataSubscribers/Labels/Kind",
    "/DataSubscribers/Labels/Value",
    "/DataSubscribers/Name",
    "/DataSubscribers/Replay",
    "/DataSubscribers/Replay/Direction",
    "/DataSubscribers/Replay/MdfChannel",
    "/DataSubscribers/Replay/MdfChannel/ChannelName",
    "/DataSubscribers/Replay/MdfChannel/ChannelPath",
    "/DataSubscribers/Replay/MdfChannel/ChannelSource",
    "/DataSubscribers/Replay/MdfChannel/GroupName",
    "/DataSubscribers/Replay/MdfChannel/GroupPath",
    "/DataSubscribers/Replay/MdfChannel/GroupSource",
    "/DataSubscribers/Replay/UseTraceSource",
    "/DataSubscribers/Topic",
    "/DataSubscribers/UseTraceSinks",
    "/Description",
    "/EthernetControllers",
    "/EthernetControllers/Name",
    "/EthernetControllers/Network",
    "/EthernetControllers/Replay",
    "/EthernetControllers/Replay/Direction",
    "/EthernetControllers/Replay/MdfChannel",
    "/EthernetControllers/Replay/MdfChannel/ChannelName",
    "/EthernetControllers/Replay/MdfChannel/ChannelPath",
    "/EthernetControllers/Replay/MdfChannel/ChannelSource",
    "/EthernetControllers/Replay/MdfChannel/GroupName",
    "/EthernetControllers/Replay/MdfChannel/GroupPath",
    "/EthernetControllers/Replay/MdfChannel/GroupSource",
    "/EthernetControllers/Replay/UseTraceSource",
    "/EthernetControllers/UseTraceSinks",
    "/Experimental",
    "/Experimental/Metrics",
    "/Experimental/Metrics/CollectFromRemote",
    "/Experimental/Metrics/Sinks",
    "/Experimental/Metrics/Sinks/Name",
    "/Experimental/Metrics/Sinks/Type",
    "/Experimental/TimeSynchronization",
    "/Experimental/TimeSynchronization/AnimationFactor",
    "/Experimental/TimeSynchronization/EnableMessageAggregation",
    "/Extensions",
    "/Extensions/SearchPathHints",
    "/FlexrayControllers",
    "/FlexRayControllers",
    "/FlexrayControllers/ClusterParameters",
    "/FlexRayControllers/ClusterParameters",
    "/FlexrayControllers/ClusterParameters/gColdstartAttempts",
    "/FlexRayControllers/ClusterParameters/gColdstartAttempts",
    "/FlexrayControllers/ClusterParameters/gCycleCountMax",
    "/FlexRayControllers/ClusterParameters/gCycleCountMax",
    "/FlexrayControllers/ClusterParameters/gdActionPointOffset",
    "/FlexRayControllers/ClusterParameters/gdActionPointOffset",
    "/FlexrayControllers/ClusterParameters/gdDynamicSlotIdlePhase",
    "/FlexRayControllers/ClusterParameters/gdDynamicSlotIdlePhase",
    "/FlexrayControllers/ClusterParameters/gdMiniSlot",
    "/FlexRayControllers/ClusterParameters/gdMiniSlot",
    "/FlexrayControllers/ClusterParameters/gdMiniSlotActionPointOffset",
    "/FlexRayControllers/ClusterParameters/gdMiniSlotActionPointOffset",
    "/FlexrayControllers/ClusterParameters/gdStaticSlot",
    "/FlexRayControllers/ClusterParameters/gdStaticSlot",
    "/FlexrayControllers/ClusterParameters/gdSymbolWindow",
    "/FlexRayControllers/ClusterParameters/gdSymbolWindow",
    "/FlexrayControllers/ClusterParameters/gdSymbolWindowActionPointOffset",
    "/FlexRayControllers/ClusterParameters/gdSymbolWindowActionPointOffset",
    "/FlexrayControllers/ClusterParameters/gdTSSTransmitter",
    "/FlexRayControllers/ClusterParameters/gdTSSTransmitter",
    "/FlexrayControllers/ClusterParameters/gdWakeupTxActive",
    "/FlexRayControllers/ClusterParameters/gdWakeupTxActive",
    "/FlexrayControllers/ClusterParameters/gdWakeupTxIdle",
    "/FlexRayControllers/ClusterParameters/gdWakeupTxIdle",
    "/FlexrayControllers/ClusterParameters/gListenNoise",
    "/FlexRayControllers/ClusterParameters/gListenNoise",
    "/FlexrayControllers/ClusterParameters/gMacroPerCycle",
    "/FlexRayControllers/ClusterParameters/gMacroPerCycle",
    "/FlexrayControllers/ClusterParameters/gMaxWithoutClockCorrectionFatal",
    "/FlexRayControllers/ClusterParameters/gMaxWithoutClockCorrectionFatal",
    "/FlexrayControllers/ClusterParameters/gMaxWithoutClockCorrectionPassive",
    "/FlexRayControllers/ClusterParameters/gMaxWithoutClockCorrectionPassive",
    "/FlexrayControllers/ClusterParameters/gNumberOfMiniSlots",
    "/FlexRayControllers/ClusterParameters/gNumberOfMiniSlots",
    "/FlexrayControllers/ClusterParameters/gNumberOfStaticSlots",
    "/FlexRayControllers/ClusterParameters/gNumberOfStaticSlots",
    "/FlexrayControllers/ClusterParameters/gPayloadLengthStatic",
    "/FlexRayControllers/ClusterParameters/gPayloadLengthStatic",
    "/FlexrayControllers/ClusterParameters/gSyncFrameIDCountMax",
    "/FlexRayControllers/ClusterParameters/gSyncFrameIDCountMax",
    "/FlexrayControllers/Name",
    "/FlexRayControllers/Name",
    "/FlexrayControllers/Network",
    "/FlexRayControllers/Network",
    "/FlexrayControllers/NodeParameters",
    "/FlexRayControllers/NodeParameters",
    "/FlexrayControllers/NodeParameters/pAllowHaltDueToClock",
    "/FlexRayControllers/NodeParameters/pAllowHaltDueToClock",
    "/FlexrayControllers/NodeParameters/pAllowPassiveToActive",
    "/FlexRayControllers/NodeParameters/pAllowPassiveToActive",
    "/FlexrayControllers/NodeParameters/pChannels",
    "/FlexRayControllers/NodeParameters/pChannels",
    "/FlexrayControllers/NodeParameters/pClusterDriftDamping",
    "/FlexRayControllers/NodeParameters/pClusterDriftDamping",
    "/FlexrayControllers/NodeParameters/pdAcceptedStartupRange",
    "/FlexRayControllers/NodeParameters/pdAcceptedStartupRange",
    "/FlexrayControllers/NodeParameters/pdListenTimeout",
    "/FlexRayControllers/NodeParameters/pdListenTimeout",
    "/FlexrayControllers/NodeParameters/pdMicrotick",
    "/FlexRayControllers/NodeParameters/pdMicrotick",
    "/FlexrayControllers/NodeParameters/pKeySlotId",
    "/FlexRayControllers/NodeParameters/pKeySlotId",
    "/FlexrayControllers/NodeParameters/pKeySlotOnlyEnabled",
    "/FlexRayControllers/NodeParameters/pKeySlotOnlyEnabled",
    "/FlexrayControllers/NodeParameters/pKeySlotUsedForStartup",
    "/FlexRayControllers/NodeParameters/pKeySlotUsedForStartup",
    "/FlexrayControllers/NodeParameters/pKeySlotUsedForSync",
    "/FlexRayControllers/NodeParameters/pKeySlotUsedForSync",
    "/FlexrayControllers/NodeParameters/pLatestTx",
    "/FlexRayControllers/NodeParameters/pLatestTx",
    "/FlexrayControllers/NodeParameters/pMacroInitialOffsetA",
    "/FlexRayControllers/NodeParameters/pMacroInitialOffsetA",
    "/FlexrayControllers/NodeParameters/pMacroInitialOffsetB",
    "/FlexRayControllers/NodeParameters/pMacroInitialOffsetB",
    "/FlexrayControllers/NodeParameters/pMicroInitialOffsetA",
    "/FlexRayControllers/NodeParameters/pMicroInitialOffsetA",
    "/FlexrayControllers/NodeParameters/pMicroInitialOffsetB",
    "/FlexRayControllers/NodeParameters/pMicroInitialOffsetB",
    "/FlexrayControllers/NodeParameters/pMicroPerCycle",
    "/FlexRayControllers/NodeParameters/pMicroPerCycle",
    "/FlexrayControllers/NodeParameters/pOffsetCorrectionOut",
    "/FlexRayControllers/NodeParameters/pOffsetCorrectionOut",
    "/FlexrayControllers/NodeParameters/pOffsetCorrectionStart",
    "/FlexRayControllers/NodeParameters/pOffsetCorrectionStart",
    "/FlexrayControllers/NodeParameters/pRateCorrectionOut",
    "/FlexRayControllers/NodeParameters/pRateCorrectionOut",
    "/FlexrayControllers/NodeParameters/pSamplesPerMicrotick",
    "/FlexRayControllers/NodeParameters/pSamplesPerMicrotick",
    "/FlexrayControllers/NodeParameters/pWakeupChannel",
    "/FlexRayControllers/NodeParameters/pWakeupChannel",
    "/FlexrayControllers/NodeParameters/pWakeupPattern",
    "/FlexRayControllers/NodeParameters/pWakeupPattern",
    "/FlexrayControllers/Replay",
    "/FlexRayControllers/Replay",
    "/FlexrayControllers/Replay/Direction",
    "/FlexRayControllers/Replay/Direction",
    "/FlexrayControllers/Replay/MdfChannel",
    "/FlexRayControllers/Replay/MdfChannel",
    "/FlexrayControllers/Replay/MdfChannel/ChannelName",
    "/FlexRayControllers/Replay/MdfChannel/ChannelName",
    "/FlexrayControllers/Replay/MdfChannel/ChannelPath",
    "/FlexRayControllers/Replay/MdfChannel/ChannelPath",
    "/FlexrayControllers/Replay/MdfChannel/ChannelSource",
    "/FlexRayControllers/Replay/MdfChannel/ChannelSource",
    "/FlexrayControllers/Replay/MdfChannel/GroupName",
    "/FlexRayControllers/Replay/MdfChannel/GroupName",
    "/FlexrayControllers/Replay/MdfChannel/GroupPath",
    "/FlexRayControllers/Replay/MdfChannel/GroupPath",
    "/FlexrayControllers/Replay/MdfChannel/GroupSource",
    "/FlexRayControllers/Replay/MdfChannel/GroupSource",
    "/FlexrayControllers/Replay/UseTraceSource",
    "/FlexRayControllers/Replay/UseTraceSource",
    "/FlexrayControllers/TxBufferConfigurations",
    "/FlexRayControllers/TxBufferConfigurations",
    "/FlexrayControllers/TxBufferConfigurations/channels",
    "/FlexRayControllers/TxBufferConfigurations/channels",
    "/FlexrayControllers/TxBufferConfigurations/headerCrc",
    "/FlexRayControllers/TxBufferConfigurations/headerCrc",
    "/FlexrayControllers/TxBufferConfigurations/offset",
    "/FlexRayControllers/TxBufferConfigurations/offset",
    "/FlexrayControllers/TxBufferConfigurations/PPindicator",
    "/FlexRayControllers/TxBufferConfigurations/PPindicator",
    "/FlexrayControllers/TxBufferConfigurations/repetition",
    "/FlexRayControllers/TxBufferConfigurations/repetition",
    "/FlexrayControllers/TxBufferConfigurations/slotId",
    "/FlexRayControllers/TxBufferConfigurations/slotId",
    "/FlexrayControllers/TxBufferConfigurations/transmissionMode",
    "/FlexRayControllers/TxBufferConfigurations/transmissionMode",
    "/FlexrayControllers/UseTraceSinks",
    "/FlexRayControllers/UseTraceSinks",
    "/HealthCheck",
    "/HealthCheck/HardResponseTimeout",
    "/HealthCheck/SoftResponseTimeout",
    "/Includes",
    "/Includes/Files",
    "/Includes/SearchPathHints",
    "/LinControllers",
    "/LinControllers/Name",
    "/LinControllers/Network",
    "/LinControllers/Replay",
    "/LinControllers/Replay/Direction",
    "/LinControllers/Replay/MdfChannel",
    "/LinControllers/Replay/MdfChannel/ChannelName",
    "/LinControllers/Replay/MdfChannel/ChannelPath",
    "/LinControllers/Replay/MdfChannel/ChannelSource",
    "/LinControllers/Replay/MdfChannel/GroupName",
    "/LinControllers/Replay/MdfChannel/GroupPath",
    "/LinControllers/Replay/MdfChannel/GroupSource",
    "/LinControllers/Replay/UseTraceSource",
    "/LinControllers/UseTraceSinks",
    "/Logging",
    "/Logging/FlushLevel",
    "/Logging/LogFromRemotes",
    "/Logging/Sinks",
    "/Logging/Sinks/Format",
    "/Logging/Sinks/Level",
    "/Logging/Sinks/LogName",
    "/Logging/Sinks/Type",
    "/Middleware",
    "/Middleware/AcceptorUris",
    "/Middleware/ConnectAttempts",
    "/Middleware/ConnectTimeoutSeconds",
    "/Middleware/EnableDomainSockets",
    "/Middleware/ExperimentalRemoteParticipantConnection",
    "/Middleware/RegistryAsFallbackProxy",
    "/Middleware/RegistryUri",
    "/Middleware/TcpNoDelay",
    "/Middleware/TcpQuickAck",
    "/Middleware/TcpReceiveBufferSize",
    "/Middleware/TcpSendBufferSize",
    "/ParticipantName",
    "/RpcClients",
    "/RpcClients/FunctionName",
    "/RpcClients/Labels",
    "/RpcClients/Labels/Key",
    "/RpcClients/Labels/Kind",
    "/RpcClients/Labels/Value",
    "/RpcClients/Name",
    "/RpcClients/Replay",
    "/RpcClients/Replay/Direction",
    "/RpcClients/Replay/MdfChannel",
    "/RpcClients/Replay/MdfChannel/ChannelName",
    "/RpcClients/Replay/MdfChannel/ChannelPath",
    "/RpcClients/Replay/MdfChannel/ChannelSource",
    "/RpcClients/Replay/MdfChannel/GroupName",
    "/RpcClients/Replay/MdfChannel/GroupPath",
    "/RpcClients/Replay/MdfChannel/GroupSource",
    "/RpcClients/Replay/UseTraceSource",
    "/RpcClients/UseTraceSinks",
    "/RpcServers",
    "/RpcServers/FunctionName",
    "/RpcServers/Labels",
    "/RpcServers/Labels/Key",
    "/RpcServers/Labels/Kind",
    "/RpcServers/Labels/Value",
    "/RpcServers/Name",
    "/RpcServers/Replay",
    "/RpcServers/Replay/Direction",
    "/RpcServers/Replay/MdfChannel",
    "/RpcServers/Replay/MdfChannel/ChannelName",
    "/RpcServers/Replay/MdfChannel/ChannelPath",
    "/RpcServers/Replay/MdfChannel/ChannelSource",
    "/RpcServers/Replay/MdfChannel/GroupName",
    "/RpcServers/Replay/MdfChannel/GroupPath",
    "/RpcServers/Replay/MdfChannel/GroupSource",
    "/RpcServers/Replay/UseTraceSource",
    "/RpcServers/UseTraceSinks",
    "/schemaVersion",
    "/SchemaVersion",
    "/Tracing",
    "/Tracing/TraceSinks",
    "/Tracing/TraceSinks/Name",
    "/Tracing/TraceSinks/OutputPath",
    "/Tracing/TraceSinks/Type",
    "/Tracing/TraceSources",
    "/Tracing/TraceSources/InputPath",
    "/Tracing/TraceSources/Name",
    "/Tracing/TraceSources/Type",

};

std::set<std::string> reservedNames; // computed from schemaPaths

auto DocumentRoot() -> std::string
{
    return schemaSeparator;
}

auto ParentPath(const std::string& elementName) -> std::string
{
    auto sep = elementName.rfind(schemaSeparator);
    if (sep == elementName.npos)
    {
        throw SilKit::ConfigurationError("elementName " + elementName + " has no parent");
    }
    else if (sep == 0)
    {
        // Special case for root lookups
        return schemaSeparator;
    }
    else
    {
        return elementName.substr(0, sep);
    }
}

auto ElementName(const std::string& elementName) -> std::string
{
    auto sep = elementName.rfind(schemaSeparator);
    if (sep == elementName.npos || sep == elementName.size())
    {
        return elementName;
    }
    return elementName.substr(sep + 1, elementName.size());
}

auto MakePath(const std::string& parentEl, const std::string& elementName) -> std::string
{
    if (elementName.empty())
    {
        return parentEl;
    }
    if (parentEl == schemaSeparator) // Special case for root lookups
    {
        return schemaSeparator + elementName;
    }
    else
    {
        return parentEl + schemaSeparator + elementName;
    }
}

bool IsReservedElementName(const std::string& queryElement)
{
    if (reservedNames.empty())
    {
        //build cache of reserved schema element names
        for (auto&& path : schemaPaths_v1)
        {
            auto elements = splitString(path, schemaSeparator);
            for (auto&& element : elements)
            {
                reservedNames.insert(element);
            }
        }
    }
    const auto elementName = ElementName(queryElement);
    return reservedNames.count(elementName);
}

bool IsSchemaPath(const std::string& path)
{
    return schemaPaths_v1.count(path) > 0;
}

using namespace SilKit::Config;

struct ValidatingVisitor
{
    std::ostream& warnings;
    std::string currentNodePath;
    std::deque<std::string> nodePaths;
    std::set<std::string> userDefinedPaths;
    ryml::Parser& parser;

    bool ok{true};

    ValidatingVisitor(ryml::Parser& parser, std::ostream& warnings)
        : warnings{warnings}
        , parser{parser}
    {
        currentNodePath = DocumentRoot();
        nodePaths.push_back(currentNodePath);
    }

    ValidatingVisitor() = delete;
    ValidatingVisitor(const ValidatingVisitor&) = delete;
    ValidatingVisitor& operator=(const ValidatingVisitor&) = delete;

    bool PathIsAlreadyDefined(const std::string& path)
    {
        auto it = userDefinedPaths.insert(path);
        return !std::get<1>(it);
    }

    auto GetCurrentLocation(ryml::ConstNodeRef node) -> std::string
    {
        auto&& location = parser.location(node);
        std::ostringstream s;

        if (location.name.empty())
        {
            s << " file " << location.name << ": ";
        }
        else
        {
            s << " string: ";
        }

        s << "line " << location.line << " column " << location.col;
        return s.str();
    }

    void push(ryml::ConstNodeRef node, ryml::id_type /* level */)
    {
        if (!node.has_key())
        {
            return;
        }

        auto nodeKey = to_string(node.key());
        auto nodePath = MakePath(currentNodePath, nodeKey);
        if (PathIsAlreadyDefined(nodePath))
        {
            warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodePath << "\""
                     << " is already defined in path \"" << currentNodePath << "\"\n";
            ok &= false;
        }

        nodePaths.push_back(nodePath);
        currentNodePath = nodePaths.back();
        return;
    }

    void pop(ryml::ConstNodeRef node, ryml::id_type /* level */)
    {
        if (node.has_key())
        {
            auto path = nodePaths.back();
            nodePaths.pop_back();
            currentNodePath = nodePaths.back();
        }
    }

    void HandleKeyVal(const ryml::ConstNodeRef& node)
    {
        auto nodeName = to_string(node.key());
        auto valuePath = MakePath(currentNodePath, nodeName);
        if (!IsSchemaPath(valuePath))
        {
            if (IsReservedElementName(valuePath))
            {
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodeName << "\""
                         << " is not a valid sub-element of schema path \"" << currentNodePath << "\"\n";
                ok &= false;
            }
            else
            {
                // We only report error if the element is a reserved keyword
                warnings << "At " << GetCurrentLocation(node) << ": Element \"" << nodeName << "\""
                         << " is being ignored. It is not a sub-element of schema path \"";
            }
        }
    }

    void HandleSeq(const ryml::ConstNodeRef& /* node */) {}

    void HandleVal(const ryml::ConstNodeRef& node)
    {
        auto value = to_string(node.val());
        auto valuePath = MakePath(currentNodePath, value);
        if (!IsSchemaPath(currentNodePath))
        {
            warnings << "At " << GetCurrentLocation(node) << ": Element \"" << value << "\""
                     << " is not a valid sub-element of schema path \"" << ParentPath(currentNodePath) << "\"\n";
            ok &= false;
        }
    }

    bool operator()(const ryml::ConstNodeRef& node, ryml::id_type /* level */)
    {
        if ((node.has_key() && (node.is_map() || node.is_seq())) || node.is_keyval())
        {
            HandleKeyVal(node);
        }
        else if (node.is_map() || node.is_seq())
        {
            HandleSeq(node);
        }
        else if (node.is_val())
        {
            HandleVal(node);
        }
        else
        {
            throw SilKit::ConfigurationError{"Unknown YAML Validation Error"};
        }
        return false;
    }

    bool IsValid() const
    {
        return ok;
    }
};

} // anonymous namespace

namespace SilKit {
namespace Config {

bool ValidateWithSchema(const std::string& yamlString, std::ostream& warnings)
{
    try
    {
        ryml::ParserOptions options{};
        options.locations(true);

        ryml::EventHandlerTree eventHandler{};

        auto parser = ryml::Parser(&eventHandler, options);
        parser.reserve_locations(100u);

        auto&& cinput = ryml::to_csubstr(yamlString);
        auto tree = ryml::parse_in_arena(&parser, cinput);

        auto root = tree.crootref();
        if (root.is_doc() && (root.is_map() || root.is_seq()))
        {
            std::string version;
            VSilKit::YamlReader reader{parser, root};
            reader.ReadKeyValue(version, "schemaVersion");
            if (version != "1")
            {
                warnings << "Cannot load schema with SchemaVersion='" << version << "'" << "\n";
                return false;
            }
        }

        ValidatingVisitor visitor{parser, warnings};
        visit_stacked(root, visitor);
        return visitor.IsValid();
    }
    catch (const std::exception& ex)
    {
        warnings << ex.what() << "\n";
        return false;
    }
}

} // namespace Config
} // namespace SilKit
