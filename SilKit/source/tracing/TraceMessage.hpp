// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// NB: type erasing in TraceMessage requires us to use concrete types
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"

#include <stdexcept>

namespace SilKit {

// helpers  to associate a TraceMessage-Type enum to a C++ type
enum class TraceMessageType
{
    EthernetFrame,
    CanFrameEvent,
    LinFrame,
    FlexrayFrameEvent,
    InvalidReplayData,
    DataMessageEvent,
};

template <TraceMessageType id>
struct TypeIdTrait
{
    static const TraceMessageType typeId = id;
};

template <class MsgT>
struct MessageTrait;

#define SILKIT_TRACING_MESSAGE(TYPENAME, TRACE_MESSAGE_TYPE_ENUMERATOR) \
    template <> \
    struct MessageTrait<TYPENAME> : TypeIdTrait<TraceMessageType::TRACE_MESSAGE_TYPE_ENUMERATOR> \
    { \
    };

// specializations for supported (C++) Types
SILKIT_TRACING_MESSAGE(Services::Ethernet::EthernetFrame, EthernetFrame)
SILKIT_TRACING_MESSAGE(Services::Can::CanFrameEvent, CanFrameEvent)
SILKIT_TRACING_MESSAGE(Services::Lin::LinFrame, LinFrame)
SILKIT_TRACING_MESSAGE(Services::Flexray::FlexrayFrameEvent, FlexrayFrameEvent)
SILKIT_TRACING_MESSAGE(Services::PubSub::DataMessageEvent, DataMessageEvent)

#undef SILKIT_TRACING_MESSAGE

class TraceMessage
{
public:
    //CTors
    TraceMessage() = delete;
    TraceMessage(const TraceMessage&) = delete;
    TraceMessage(const TraceMessage&&) = delete;
    TraceMessage operator=(const TraceMessage&) = delete;
    TraceMessage& operator=(const TraceMessage&&) = delete;

    template <typename MsgT>
    explicit TraceMessage(const MsgT& msg)
        : _type{getTypeId<MsgT>()}
        , _value{reinterpret_cast<const void*>(&msg)}
    {
    }

    TraceMessageType Type() const
    {
        return _type;
    }

    template <typename MsgT>
    const MsgT& Get() const
    {
        const auto tag = getTypeId<std::decay_t<MsgT>>();
        if (tag != _type)
        {
            throw SilKitError("TraceMessage::Get() Requested type does not match stored type.");
        }

        return *(reinterpret_cast<const MsgT*>(_value));
    }

private:
    template <typename MsgT>
    constexpr TraceMessageType getTypeId() const
    {
        return MessageTrait<MsgT>::typeId;
    }

    TraceMessageType _type;
    const void* _value;
};

} // namespace SilKit
