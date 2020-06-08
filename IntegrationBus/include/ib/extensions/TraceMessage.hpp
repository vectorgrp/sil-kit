// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

// NB: type erasing in TraceMessage requires us to use concrete types
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/fr/FrDatatypes.hpp"

namespace ib {
namespace extensions {


// helpers  to associate a TraceMessage-Type enum to a C++ type
enum class TraceMessageType
{
    EthFrame
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

template<class MsgT> struct MessageTrait;
// specializations for supported (C++) Types
template<> struct MessageTrait<sim::eth::EthFrame> : TypeIdTrait<TraceMessageType::EthFrame> {};
template<> struct MessageTrait<sim::can::CanMessage> : TypeIdTrait<TraceMessageType::CanMessage> {};
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
    TraceMessage(const TraceMessage&&) = delete;
    TraceMessage operator=(const TraceMessage&) = delete;
    TraceMessage& operator=(const TraceMessage&&) = delete;

    template<typename MsgT>
    TraceMessage(const MsgT& msg)
        : _type{getTypeId<MsgT>()}
        , _value{reinterpret_cast<const void*>(&msg)}
    {
    }

    TraceMessageType Type() const
    {
        return _type;
    }

    template<typename MsgT>
    const MsgT& Get() const
    {
        const auto tag = getTypeId<std::decay_t<MsgT>>();
        if (tag != _type)
        {
            throw std::runtime_error("TraceMessage::Get() Requested type does not match stored type.");
        }

        return *(reinterpret_cast<const MsgT*>(_value));
    }

private:
    template<typename MsgT>
    constexpr TraceMessageType getTypeId() const
    {
        return MessageTrait<MsgT>::typeId;
    }

    TraceMessageType _type;
    const void* _value;
};



} //end namespace extensions
} //end namespace ib
