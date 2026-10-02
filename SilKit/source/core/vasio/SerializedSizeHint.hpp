// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>

#include "wire/can/WireCanMessages.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"
#include "wire/flexray/WireFlexrayMessages.hpp"
#include "wire/pubsub/WireDataMessages.hpp"
#include "wire/rpc/WireRpcMessages.hpp"

namespace SilKit {
namespace Core {

//! Network headers plus room for the fixed size fields of small messages.
constexpr size_t SerializedSizeHintFixedOverhead{128};

/*! \brief Capacity hint for the serialization buffer of a message.
 *
 * For payload carrying messages the hint must cover the payload, otherwise a field written after
 * it can trigger a reallocation that copies the payload.
 */
template <typename MessageT>
struct SerializedSizeHint
{
    static auto Of(const MessageT& /*message*/) -> size_t
    {
        return SerializedSizeHintFixedOverhead;
    }
};

#define DefineSerializedSizeHintForPayload(MessageType, PayloadExpression) \
    template <> \
    struct SerializedSizeHint<MessageType> \
    { \
        static auto Of(const MessageType& message) -> size_t \
        { \
            return SerializedSizeHintFixedOverhead + (PayloadExpression).size(); \
        } \
    }

DefineSerializedSizeHintForPayload(SilKit::Services::Can::WireCanFrameEvent, message.frame.dataField.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::Ethernet::WireEthernetFrameEvent, message.frame.raw.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::PubSub::WireDataMessageEvent, message.data.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::Flexray::WireFlexrayFrameEvent, message.frame.payload.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::Flexray::WireFlexrayFrameTransmitEvent,
                                   message.frame.payload.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::Flexray::WireFlexrayTxBufferUpdate, message.payload.AsSpan());
DefineSerializedSizeHintForPayload(SilKit::Services::Rpc::FunctionCall, message.data);
DefineSerializedSizeHintForPayload(SilKit::Services::Rpc::FunctionCallResponse, message.data);

#undef DefineSerializedSizeHintForPayload

} // namespace Core
} // namespace SilKit
