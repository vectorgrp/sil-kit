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

#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <cctype>

#include "silkit/participant/exception.hpp"
#include "silkit/SilKitMacros.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "Optional.hpp"
#include "StringHelpers.hpp"

namespace SilKit {
namespace Config {

inline namespace v1 {

// ================================================================================
//  Shared configuration declarations
// ================================================================================

enum class NetworkType
{
    Undefined,
    Invalid,
    CAN,
    LIN,
    Ethernet,
    FlexRay,
    Data,
    RPC
};
inline auto to_string(NetworkType networkType) -> std::string;

// ================================================================================
//  Message aggregation declarations
// ================================================================================

enum class Aggregation : uint32_t
{
    Off = 0, // disable aggregation
    On = 1, // enable aggregation for time synchronization (synchronous and asynchronous case)
    Auto = 2 // enable aggregation for time synchronization (synchronous case)
};

inline std::string to_string(const Aggregation& aggregation);
inline Aggregation from_string(const std::string& aggregationStr);
inline std::ostream& operator<<(std::ostream& out, const Aggregation& aggregation);

// ================================================================================
//  Logging service
// ================================================================================

struct Sink
{
    enum class Type : uint8_t
    {
        Remote,
        Stdout,
        File
    };


    enum class Format : uint8_t
    {
        Simple,
        Json
    };

    Format format{Format::Simple};
    Type type{Type::Remote};
    Services::Logging::Level level{Services::Logging::Level::Info};
    std::string logName;
};

//! \brief Logger service
struct Logging
{
    bool logFromRemotes{false};
    Services::Logging::Level flushLevel{Services::Logging::Level::Off};
    std::vector<Sink> sinks;
};

// ================================================================================
//  Tracing service
// ================================================================================

struct TraceSink
{
    enum class Type
    {
        Undefined,
        PcapFile,
        PcapPipe,
        Mdf4File
    };

    Type type{Type::Undefined};
    std::string name;
    std::string outputPath;
};

struct TraceSource
{
    enum class Type
    {
        Undefined,
        PcapFile,
        Mdf4File
    };

    Type type{Type::Undefined};
    std::string name;
    std::string inputPath;
};

//! MdfChannel identification for replaying, refer to ASAM MDF 4.1 Specification, Chapter 5.4.3
struct MdfChannel
{
    // A user supplied empty string in the configuration is valid
    SilKit::Util::Optional<std::string> channelName; //!< maps to MDF cn_tx_name
    SilKit::Util::Optional<std::string> channelSource; //!< maps to MDF si_tx_name of cn_si_source
    SilKit::Util::Optional<std::string> channelPath; //!< maps to MDF si_tx_path of cn_si_source

    SilKit::Util::Optional<std::string> groupName; //!< maps to MDF cg_tx_name
    SilKit::Util::Optional<std::string> groupSource; //!< maps to MDF si_tx_name of cg_si_acq_source
    SilKit::Util::Optional<std::string> groupPath; //!< maps to MDF si_tx_path of cn_si_acq_source
};

struct Replay
{
    std::string useTraceSource;

    enum class Direction
    {
        Undefined,
        Send,
        Receive,
        Both,
    };
    Direction direction{Direction::Undefined};
    MdfChannel mdfChannel;
};

struct SimulatedNetwork
{
    // This method allows for generic usage, along with ControllerConfigTs
    auto GetNetworkType() -> NetworkType
    {
        return type;
    }
    std::string name;
    NetworkType type{NetworkType::Undefined};
    std::vector<std::string> useTraceSinks;
    Replay replay;
};

inline bool operator==(const Sink& lhs, const Sink& rhs);
inline bool operator<(const Sink& lhs, const Sink& rhs);
inline bool operator>(const Sink& lhs, const Sink& rhs);

inline bool operator==(const Logging& lhs, const Logging& rhs);
inline bool operator==(const TraceSink& lhs, const TraceSink& rhs);
inline bool operator==(const TraceSource& lhs, const TraceSource& rhs);
inline bool operator==(const Replay& lhs, const Replay& rhs);
inline bool operator==(const MdfChannel& lhs, const MdfChannel& rhs);

inline auto to_string(TraceSink::Type sinkType) -> std::string;

// ================================================================================
//  Inline Implementations
// ================================================================================

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
    default:
        throw SilKit::TypeConversionError{"Invalid SinkType"};
    }
}

auto to_string(NetworkType networkType) -> std::string
{
    switch (networkType)
    {
    case NetworkType::Undefined:
        return "Undefined";
    case NetworkType::Invalid:
        return "Invalid";
    case NetworkType::CAN:
        return "CAN";
    case NetworkType::LIN:
        return "LIN";
    case NetworkType::Ethernet:
        return "Ethernet";
    case NetworkType::FlexRay:
        return "FlexRay";
    case NetworkType::Data:
        return "Data";
    case NetworkType::RPC:
        return "RPC";
    default:
        return "Unknown";
    }
}

bool operator==(const Sink& lhs, const Sink& rhs)
{
    return lhs.type == rhs.type && lhs.level == rhs.level && lhs.format == rhs.format && lhs.logName == rhs.logName;
}

bool operator<(const Sink& lhs, const Sink& rhs)
{
    return std::make_tuple(lhs.type, lhs.logName) < std::make_tuple(rhs.type, rhs.logName);
}

bool operator>(const Sink& lhs, const Sink& rhs)
{
    return rhs < lhs;
}

bool operator==(const Logging& lhs, const Logging& rhs)
{
    return lhs.logFromRemotes == rhs.logFromRemotes && lhs.flushLevel == rhs.flushLevel && lhs.sinks == rhs.sinks;
}

bool operator==(const TraceSink& lhs, const TraceSink& rhs)
{
    return lhs.name == rhs.name && lhs.outputPath == rhs.outputPath && lhs.type == rhs.type;
}

bool operator==(const TraceSource& lhs, const TraceSource& rhs)
{
    return lhs.inputPath == rhs.inputPath && lhs.type == rhs.type && lhs.name == rhs.name;
}

bool operator==(const Replay& lhs, const Replay& rhs)
{
    return lhs.useTraceSource == rhs.useTraceSource && lhs.direction == rhs.direction
           && lhs.mdfChannel == rhs.mdfChannel;
}

bool operator==(const MdfChannel& lhs, const MdfChannel& rhs)
{
    return lhs.channelName == rhs.channelName && lhs.channelSource == rhs.channelSource
           && lhs.channelPath == rhs.channelPath && lhs.groupName == rhs.groupName && lhs.groupSource == rhs.groupSource
           && lhs.groupPath == rhs.groupPath;
}

std::string to_string(const Aggregation& aggregation)
{
    std::stringstream outStream;
    outStream << aggregation;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& outStream, const Aggregation& aggregation)
{
    switch (aggregation)
    {
    case Aggregation::Off:
        outStream << "Off";
        break;
    case Aggregation::On:
        outStream << "On";
        break;
    case Aggregation::Auto:
        outStream << "Auto";
        break;
    default:
        outStream << "Invalid Aggregation";
    }
    return outStream;
}

inline Aggregation from_string(const std::string& aggregationStr)
{
    auto aggregation = SilKit::Util::LowerCase(aggregationStr);
    if (aggregation == "off")
        return Aggregation::Off;
    if (aggregation == "on")
        return Aggregation::On;
    if (aggregation == "auto")
        return Aggregation::Auto;
    // default to Auto
    return Aggregation::Auto;
}

} // namespace v1

} // namespace Config
} // namespace SilKit
