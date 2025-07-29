// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"

namespace SilKit {
namespace Core {

////////////////////////////////////////////////////////////////////////////////
// Early Protocol Handshake and Initial Service Subscription
// VAsioMsgKind: SilKitRegistryMessage
////////////////////////////////////////////////////////////////////////////////

// Extract the message size (first element in wire format message)
auto ExtractMessageSize(MessageBuffer& buffer) -> uint32_t;
// Extract the message kind tag (second element in wire format message)
auto ExtractMessageKind(MessageBuffer& buffer) -> VAsioMsgKind;
// Extract the registry message kind tag (third element for handshake messages)
auto ExtractRegistryMessageKind(MessageBuffer& buffer) -> RegistryMessageKind;

auto PeekRegistryMessageHeader(MessageBuffer& buffer) -> RegistryMsgHeader;
auto PeekProxyMessageHeader(MessageBuffer& buffer) -> ProxyMessageHeader;

auto ExtractEndpointId(MessageBuffer& buffer) -> EndpointId;
auto ExtractEndpointAddress(MessageBuffer& buffer) -> EndpointAddress;

//! Handshake: Serialize ParticipantAnnouncementReply (contains remote peer's protocol version)
//  VAsioMsgKind: SilKitRegistryMessage
void Serialize(MessageBuffer& buffer, const ParticipantAnnouncement& announcement);
void Serialize(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply);
void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber);
void Serialize(MessageBuffer& buffer, const SubscriptionAcknowledge& msg);
void Serialize(MessageBuffer& buffer, const KnownParticipants& msg);
void Serialize(MessageBuffer& buffer, const ProxyMessage& msg);
void Serialize(MessageBuffer& buffer, const RemoteParticipantConnectRequest& msg);

void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out);
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncementReply& out);
void Deserialize(MessageBuffer&, VAsioMsgSubscriber&);
void Deserialize(MessageBuffer&, SubscriptionAcknowledge&);
void Deserialize(MessageBuffer& buffer, KnownParticipants& out);
void Deserialize(MessageBuffer& buffer, ProxyMessage& out);
void Deserialize(MessageBuffer& buffer, RemoteParticipantConnectRequest& out);

} // namespace Core
} // namespace SilKit
