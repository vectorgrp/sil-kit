// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
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

/*! \brief Capacity hint for the serialization buffer of a message.
 *
 * Reserving the full serialized size up front keeps the whole serialization free of
 * reallocations. That matters most for the payload carrying messages: without an accurate hint,
 * a field written after a large payload can trigger a reallocation that copies the payload.
 *
 * The hint may over- but must never systematically under-estimate for payload carrying types.
 * It is only a capacity hint, so an inaccurate value costs performance, never correctness.
 */
template <typename MessageT>
struct SerializedSizeHint
{
    //! Network headers plus room for the fixed size fields of small messages.
    static constexpr size_t kFixedOverhead = 128;

    static auto Of(const MessageT& /*message*/) -> size_t
    {
        return kFixedOverhead;
    }
};

#define DefineSerializedSizeHintForPayload(MessageType, PayloadExpression) \
    template <> \
    struct SerializedSizeHint<MessageType> \
    { \
        static constexpr size_t kFixedOverhead = 128; \
        static auto Of(const MessageType& message) -> size_t \
        { \
            return kFixedOverhead + (PayloadExpression).size(); \
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
