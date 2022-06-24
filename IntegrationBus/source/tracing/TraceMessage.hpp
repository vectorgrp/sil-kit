// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

// NB: type erasing in TraceMessage requires us to use concrete types
#include "ib/sim/eth/EthernetDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/fr/FlexrayDatatypes.hpp"

#include <stdexcept>

namespace ib {
namespace extensions {


// helpers  to associate a TraceMessage-Type enum to a C++ type
enum class TraceMessageType
{
    EthernetFrame,
    CanFrameEvent,
    LinFrame,
    FlexrayFrameEvent,
    InvalidReplayData,
    //TODO FlexraySymbolEvent, FlexrayPocStatusEvent, FlexrayTxBufferConfigUpdate, FlexrayTxBufferUpdate ?
};

template<TraceMessageType id>
struct TypeIdTrait
{
    static const TraceMessageType typeId = id;
};

template<class MsgT> struct MessageTrait;
// specializations for supported (C++) Types
template<> struct MessageTrait<sim::eth::EthernetFrame> : TypeIdTrait<TraceMessageType::EthernetFrame> {};
template<> struct MessageTrait<sim::can::CanFrameEvent> : TypeIdTrait<TraceMessageType::CanFrameEvent> {};
template<> struct MessageTrait<sim::lin::LinFrame> : TypeIdTrait<TraceMessageType::LinFrame> {};
template<> struct MessageTrait<sim::fr::FlexrayFrameEvent> : TypeIdTrait<TraceMessageType::FlexrayFrameEvent> {};

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
