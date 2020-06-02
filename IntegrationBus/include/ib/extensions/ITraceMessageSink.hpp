// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <chrono>
#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/fwd_decl.hpp"
#include "ib/cfg/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"

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

    //Tracing methods for simulation message types
    // the controller and bus names are used to encode meta infos in the trace file

    // EthernetController message type
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id, //!< the ID is used to identify the controller this message is from
        std::chrono::nanoseconds timestamp,
        const sim::eth::EthFrame& message) = 0;

    // CanController message Type
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::can::CanMessage& message) = 0;

    // LinController message Type
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::lin::Frame& message) = 0;

    // Generic Publisher / Subscriber message type
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::generic::GenericMessage& message) = 0;

    // the various message types of the I/O Port specialisations
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::io::AnalogIoMessage& message) = 0;

    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::io::DigitalIoMessage& message) = 0;

    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::io::PatternIoMessage& message) = 0;

    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::io::PwmIoMessage& message) = 0;

    // FlexrayController message types
    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const sim::fr::FrMessage& message) = 0;
    //TODO do we need to add FrSymbol, PocStatus, TxBufferConfigUpdate, TxBufferUpdate ?

};

} //end namespace extensions
} //end namespace ib
