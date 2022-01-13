// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Config.hpp"

#include <string>
#include <ostream>

#include "ib/exception.hpp"

namespace ib {

template <typename T>
auto from_string(const std::string& value) -> T;

template <>
inline auto from_string<cfg::Middleware>(const std::string& value) -> cfg::Middleware;

namespace cfg {

inline auto to_string(SyncType syncType) -> std::string;
inline auto operator<<(std::ostream& out, SyncType) -> std::ostream&;

inline auto to_string(TimeSync::SyncPolicy syncPolicy) -> std::string;
inline auto to_string(TraceSink::Type sinkType) -> std::string;

inline auto to_string(Middleware middleware) -> std::string;
inline auto operator<<(std::ostream& out, Middleware middleware) -> std::ostream&;

namespace FastRtps {

inline auto to_string(DiscoveryType discoveryType) -> std::string;
inline auto operator<<(std::ostream& out, DiscoveryType discoveryType) -> std::ostream&;

} // namespace FastRtps
} // namespace cfg

// ================================================================================
//  Inline Implementations
// ================================================================================

template <>
auto from_string<cfg::Middleware>(const std::string& value) -> cfg::Middleware
{
    if (value == "VAsio")
    {
        return cfg::Middleware::VAsio;
    }
    if (value == "FastRTPS")
    {
        return cfg::Middleware::FastRTPS;
    }
    if (value == "NotConfigured")
    {
        return cfg::Middleware::NotConfigured;
    }
    throw type_conversion_error("Invalid middleware: " + value);
}

namespace cfg {

auto to_string(SyncType syncType) -> std::string
{
    switch (syncType)
    {
    case SyncType::DistributedTimeQuantum:
        return "DistributedTimeQuantum";
    case SyncType::DiscreteEvent:
        return "DiscreteEvent";
    case SyncType::TimeQuantum:
        return "TimeQuantum";
    case SyncType::DiscreteTime:
        return "DiscreteTime";
    case SyncType::DiscreteTimePassive:
        return "DiscreteTimePassive";
    case SyncType::Unsynchronized:
        return "Unsynchronized";
    case SyncType::Synchronized:
        return "Synchronized";
    }
    throw ib::type_conversion_error{};
}

std::ostream& operator<<(std::ostream& out, SyncType syncType)
{
    try
    {
        return out << to_string(syncType);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "SyncType{" << static_cast<uint32_t>(syncType) << "}";
    }
}

auto to_string(TimeSync::SyncPolicy syncPolicy) -> std::string
{
    switch (syncPolicy)
    {
    case TimeSync::SyncPolicy::Loose:
        return "Loose";
    case TimeSync::SyncPolicy::Strict:
        return "Strict";
    }
    throw ib::type_conversion_error{};
}

auto to_string(TraceSink::Type sinkType) -> std::string
{
    switch (sinkType)
    {
    case TraceSink::Type::Mdf4File:
        return "Mdf4File";
    case TraceSink::Type::PcapFile:
        return "PcapFile";
    case TraceSink::Type::PcapPipe:
        return "PcapPipe";
    case TraceSink::Type::Undefined:
        return "Undefined";
    }
    throw ib::type_conversion_error{"Invalid SinkType"};
}

inline auto to_string(Middleware middleware) -> std::string
{
    switch (middleware)
    {
    case Middleware::FastRTPS:
        return "FastRTPS";
    case Middleware::VAsio:
        return "VAsio";
    case Middleware::NotConfigured:
        return "NotConfigured";
    }
    throw ib::type_conversion_error{};
}

inline auto operator<<(std::ostream& out, Middleware middleware) -> std::ostream&
{
    return out << to_string(middleware);
}

namespace FastRtps {

auto to_string(DiscoveryType discoveryType) -> std::string
{
    switch (discoveryType)
    {
    case DiscoveryType::Local:
        return "Local";
    case DiscoveryType::Multicast:
        return "Multicast";
    case DiscoveryType::Unicast:
        return "Unicast";
    case DiscoveryType::ConfigFile:
        return "ConfigFile"; 
    }
    throw ib::type_conversion_error{};
}

auto operator<<(std::ostream& out, DiscoveryType discoveryType) -> std::ostream&
{
    return out << to_string(discoveryType);
}

} // namespace FastRtps

} // namespace cfg
} // namespace ib
