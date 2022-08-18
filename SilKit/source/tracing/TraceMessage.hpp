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
    #ifdef SILKIT_HAVE_TRACING
    FlexraySymbolEvent, FlexrayPocStatusEvent, FlexrayTxBufferConfigUpdate, FlexrayTxBufferUpdate
    #endif
};

template<TraceMessageType id>
struct TypeIdTrait
{
    static const TraceMessageType typeId = id;
};

template<class MsgT> struct MessageTrait;
// specializations for supported (C++) Types
template<> struct MessageTrait<Services::Ethernet::EthernetFrame> : TypeIdTrait<TraceMessageType::EthernetFrame> {};
template<> struct MessageTrait<Services::Can::CanFrameEvent> : TypeIdTrait<TraceMessageType::CanFrameEvent> {};
template<> struct MessageTrait<Services::Lin::LinFrame> : TypeIdTrait<TraceMessageType::LinFrame> {};
template<> struct MessageTrait<Services::Flexray::FlexrayFrameEvent> : TypeIdTrait<TraceMessageType::FlexrayFrameEvent> {};

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
            throw SilKitError("TraceMessage::Get() Requested type does not match stored type.");
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




} //end namespace SilKit
