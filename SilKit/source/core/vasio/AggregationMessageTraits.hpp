// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/vasio/VAsioDatatypes.hpp"

#include "core/internal/OrchestrationDatatypes.hpp"

#include "wire/can/WireCanMessages.hpp"
#include "wire/pubsub/WireDataMessages.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"
#include "wire/rpc/WireRpcMessages.hpp"

namespace SilKit {
namespace Core {

/// default comprising VAsio data types and service data types
template <typename MessageT>
inline constexpr auto aggregationKind() -> MessageAggregationKind
{
    return MessageAggregationKind::Other;
}

/// messages signalizing the end of a time step
template <>
inline constexpr auto aggregationKind<SilKit::Services::Orchestration::NextSimTask>() -> MessageAggregationKind
{
    return MessageAggregationKind::FlushAggregationMessage;
}
template <>
inline constexpr auto aggregationKind<SilKit::Services::Orchestration::ParticipantStatus>() -> MessageAggregationKind
{
    return MessageAggregationKind::FlushAggregationMessage;
}

/// messages containing user data/payload (to be aggregated)
// PubSub
template <>
inline constexpr auto aggregationKind<SilKit::Services::PubSub::WireDataMessageEvent>() -> MessageAggregationKind
{
    return MessageAggregationKind::UserDataMessage;
}
// RPC
template <>
inline constexpr auto aggregationKind<SilKit::Services::Rpc::FunctionCall>() -> MessageAggregationKind
{
    return MessageAggregationKind::UserDataMessage;
}
template <>
inline constexpr auto aggregationKind<SilKit::Services::Rpc::FunctionCallResponse>() -> MessageAggregationKind
{
    return MessageAggregationKind::UserDataMessage;
}
// CAN
template <>
inline constexpr auto aggregationKind<SilKit::Services::Can::WireCanFrameEvent>() -> MessageAggregationKind
{
    return MessageAggregationKind::UserDataMessage;
}
// Ethernet
template <>
inline constexpr auto aggregationKind<SilKit::Services::Ethernet::WireEthernetFrameEvent>() -> MessageAggregationKind
{
    return MessageAggregationKind::UserDataMessage;
}

} // namespace Core
} // namespace SilKit