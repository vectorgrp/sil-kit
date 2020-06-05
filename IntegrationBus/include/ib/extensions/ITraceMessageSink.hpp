// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <chrono>
#include <tuple>

#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"

#include "ib/extensions/TraceMessage.hpp"

namespace ib {
namespace extensions {

//! \brief Direction indicates whether the message was received or transmitted.
enum class Direction
{
	Receive,
	Send
};

//! \brief SinkType specifies the type of the output backend to use for permanent storage.
enum class SinkType
{
	PcapFile,
	PcapNamedPipe,
	Mdf4File
};

//! \brief Messages traces are written to a message sink.
//         It might be provided by an IB extension or built into IB

class ITraceMessageSink
{
public:
    virtual ~ITraceMessageSink() = default;

   
    virtual void Open(SinkType type, const std::string& outputPath) = 0;
    virtual void Close() = 0;
    virtual auto GetLogger() const -> mw::logging::ILogger* = 0;
    virtual auto Name() const -> const std::string& = 0;

    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id, //!< the ID is used to identify the controller this message is from
        std::chrono::nanoseconds timestamp,
        const TraceMessage& message) = 0;
};

} //end namespace extensions
} //end namespace ib
