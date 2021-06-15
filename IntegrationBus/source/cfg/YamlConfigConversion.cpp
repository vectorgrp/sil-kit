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
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, CanController& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const LinController& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, LinController& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const EthernetController& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, EthernetController& obj)
{
    return false;
}

template<>
Node VibConversion::encode(const FlexrayController& obj)
{
    Node node;
    return node;
}
template<>
bool VibConversion::decode(const Node& node, FlexrayController& obj)
{
    return false;
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
