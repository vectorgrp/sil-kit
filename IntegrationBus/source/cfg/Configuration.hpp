// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ib/exception.hpp"
#include "ib/IbMacros.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"

#include "Optional.hpp"

namespace ib {
namespace cfg {

inline namespace v1 {

namespace datatypes {

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
    GenericMessage,
    DataMessage
};

// ================================================================================
//  Logging service
// ================================================================================

struct Sink
{
    enum class Type
    {
        Remote,
        Stdout,
        File
    };

    ib::util::Optional<Type> type{ Type::Remote };
    ib::util::Optional<mw::logging::Level> level{ mw::logging::Level::Info };
    ib::util::Optional<std::string> logName;
};

//! \brief Logger service
struct Logging
{
    ib::util::Optional<bool> logFromRemotes{ false };
    ib::util::Optional<mw::logging::Level> flushLevel{ mw::logging::Level::Off };
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

    ib::util::Optional<Type> type{ Type::Undefined };
    ib::util::Optional<std::string> name;
    ib::util::Optional<std::string> outputPath;
    //ib::util::Optional<bool> enabled{ true };
};

struct TraceSource
{
    enum class Type
    {
        Undefined,
        PcapFile,
        Mdf4File
    };

    ib::util::Optional<Type> type{ Type::Undefined };
    ib::util::Optional<std::string> name;
    ib::util::Optional<std::string> inputPath;
    //ib::util::Optional<bool> enabled{ true };
};

//!< MdfChannel identification for replaying, refer to ASAM MDF 4.1 Specification, Chapter 5.4.3
struct MdfChannel
{
    // A user supplied empty string in the configuration is valid
    ib::util::Optional<std::string> channelName; //!< maps to MDF cn_tx_name
    ib::util::Optional<std::string> channelSource; //!< maps to MDF si_tx_name of cn_si_source
    ib::util::Optional<std::string> channelPath; //!< maps to MDF si_tx_path of cn_si_source

    ib::util::Optional<std::string> groupName; //!< maps to MDF cg_tx_name
    ib::util::Optional<std::string> groupSource; //!< maps to MDF si_tx_name of cg_si_acq_source
    ib::util::Optional<std::string> groupPath; //!< maps to MDF si_tx_path of cn_si_acq_source
};

struct Replay
{
    ib::util::Optional<std::string> useTraceSource;

    enum class Direction
    {
        Undefined,
        Send,
        Receive,
        Both,
    };
    ib::util::Optional<Direction> direction{ Direction::Undefined };
    std::vector<std::string> filterMessage;
    ib::util::Optional<MdfChannel> mdfChannel;
};

bool operator==(const Sink& lhs, const Sink& rhs);
bool operator==(const Logging& lhs, const Logging& rhs);
bool operator==(const TraceSink& lhs, const TraceSink& rhs);
bool operator==(const TraceSource& lhs, const TraceSource& rhs);
bool operator==(const Replay& lhs, const Replay& rhs);
bool operator==(const MdfChannel& lhs, const MdfChannel& rhs);

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
    }
    throw ib::type_conversion_error{ "Invalid SinkType" };
}

} // namespace datatypes

} // inline namespace v1

} // namespace cfg
} // namespace ib
