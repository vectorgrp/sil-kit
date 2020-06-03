// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <chrono>
#include <tuple>

#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"


// NB: type erasing in TraceMessage requires us to use concrete types
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/fr/FrDatatypes.hpp"

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


// helpers  to associate a TraceMessage-Type enum to a C++ type
enum class TraceMessageType
{
    Invalid
    ,EthFrame
    ,CanMessage
    ,LinFrame
    ,GenericMessage
    ,AnlogIoMessage
    ,DigitalIoMessage
    ,PatternIoMessage
    ,PwmIoMessage
    ,FrMessage
    //TODO FrSymbol, PocStatus, TxBufferConfigUpdate, TxBufferUpdate ?
};

template<TraceMessageType id>
struct TypeIdTrait
{
    static const TraceMessageType typeId = id;
};

// specializations for supported (C++) Types
template<class MsgT> struct MessageTrait : TypeIdTrait<TraceMessageType::Invalid> {};
template<> struct MessageTrait<sim::eth::EthFrame> : TypeIdTrait<TraceMessageType::EthFrame> {};
template<> struct MessageTrait<sim::lin::Frame> : TypeIdTrait<TraceMessageType::LinFrame> {};
template<> struct MessageTrait<sim::generic::GenericMessage> : TypeIdTrait<TraceMessageType::GenericMessage> {};
template<> struct MessageTrait<sim::io::AnalogIoMessage> : TypeIdTrait<TraceMessageType::AnlogIoMessage> {};
template<> struct MessageTrait<sim::io::DigitalIoMessage> : TypeIdTrait<TraceMessageType::DigitalIoMessage> {};
template<> struct MessageTrait<sim::io::PatternIoMessage> : TypeIdTrait<TraceMessageType::PatternIoMessage> {};
template<> struct MessageTrait<sim::io::PwmIoMessage> : TypeIdTrait<TraceMessageType::PwmIoMessage> {};
template<> struct MessageTrait<sim::fr::FrMessage> : TypeIdTrait<TraceMessageType::FrMessage> {};

class TraceMessage
{
public:
    //CTors
    TraceMessage() = delete;
    TraceMessage(const TraceMessage&) = delete;
    TraceMessage operator=(const TraceMessage&) = delete;

    template<typename MsgT>
    TraceMessage(const MsgT& msg)
        : _type{getTypeId<MsgT>()}
        , _value{reinterpret_cast<const void*>(&msg)}
    {
    }

    TraceMessageType QueryType() const
    {
        return _type;
    }

    template<typename MsgT>
    const MsgT& Get() const
    {
        const auto tag = getTypeId<MsgT>();
        if (tag != _type)
        {
            throw std::runtime_error("TraceMessage: Unsupported type used as template argument");
        }

        return *(reinterpret_cast<const MsgT*>(_value));
    }

private:
    template<typename MsgT>
    constexpr TraceMessageType getTypeId() const
    {
        static_assert(MessageTrait<MsgT>::typeId != TraceMessageType::Invalid,
            "Unknown Message type used!");
        return MessageTrait<MsgT>::typeId;
    }

    TraceMessageType _type;
    const void* _value;
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
