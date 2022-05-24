// Copyright (c) Vector Informatik GmbH. All rights reserved.
// VAsio network protocol
// Use the Deserialize/Serialize functions for versioned
// (backward compatible) access to the protocol wire format

#pragma once

#include "EndpointAddress.hpp"
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"
#include "MessageBuffer.hpp"
#include "VAsioProtocolVersion.hpp"

//fowards
#include "ib/sim/can/fwd_decl.hpp"
#include "ib/sim/eth/fwd_decl.hpp"
#include "ib/sim/fr/fwd_decl.hpp"
#include "ib/sim/lin/fwd_decl.hpp"
#include "ib/sim/data/fwd_decl.hpp"
#include "ib/sim/rpc/fwd_decl.hpp"
#include "ib/mw/sync/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"
//concrete
#include "ServiceDatatypes.hpp"

// SerDes helpers to reduce boiler plate and encapsulate the VAsio's network wire format

namespace ib {
namespace mw {
////////////////////////////////////////////////////////////////////////////////
// Early Protocol Handshake and Initial Service Subscription
// VAsioMsgKind: IbRegistryMessage
////////////////////////////////////////////////////////////////////////////////

// Extract the message size (first element in wire format message)
auto ExtractMessageSize(MessageBuffer& buffer) -> uint32_t;
// Extract the message kind tag (second element in wire format message)
auto ExtractMessageKind(MessageBuffer& buffer) -> VAsioMsgKind;
// Extract the registry message kind tag (third element for handshake messages)
auto ExtractRegistryMessageKind(MessageBuffer& buffer) -> RegistryMessageKind;

auto PeekRegistryMessageHeader(const MessageBuffer& buffer) -> RegistryMsgHeader;
auto ExtractEndpointId(MessageBuffer& buffer) ->EndpointId;
auto ExtractEndpointAddress(MessageBuffer& buffer) ->EndpointAddress;

//! Handshake: Serialize ParticipantAnnouncementReply (contains remote peer's protocol version)
//  VAsioMsgKind: IbRegistryMessage
void Serialize(MessageBuffer buffer, const ParticipantAnnouncement& announcement);
void Serialize(MessageBuffer buffer, const ParticipantAnnouncementReply& reply);
void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber);
void Serialize(MessageBuffer buffer, const SubscriptionAcknowledge& msg);
void Serialize(MessageBuffer& buffer, const KnownParticipants& msg);

void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out);
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out);
void Deserialize(MessageBuffer&, VAsioMsgSubscriber&);
void Deserialize(MessageBuffer&, SubscriptionAcknowledge&);
void Deserialize(MessageBuffer& buffer,KnownParticipants& out);

//! Check if we support ser/des for the given protocol version
bool ProtocolVersionSupported(const RegistryMsgHeader& header);

////////////////////////////////////////////////////////////////////////////////
// Services for established connections
// VAsioMsgKind: IbSimMsg, IbMwMsg (deprecated??)
////////////////////////////////////////////////////////////////////////////////

// Deserializers which exclude the network trailers: we assume that the 
// VAsioMsgKind, EndpointId, and EndpointAddress members have been extracted.
void Deserialize(MessageBuffer& buffer, logging::LogMsg& out);
void Deserialize(MessageBuffer& buffer, sync::ParticipantCommand& out);
void Deserialize(MessageBuffer& buffer, sync::SystemCommand& out);
void Deserialize(MessageBuffer& buffer, sync::ParticipantStatus& out);
void Deserialize(MessageBuffer& buffer, sync::ExpectedParticipants& out);
void Deserialize(MessageBuffer& buffer, sync::NextSimTask& out);
void Deserialize(MessageBuffer& buffer, sim::data::DataMessageEvent& out);
void Deserialize(MessageBuffer& buffer, sim::rpc::FunctionCall& out);
void Deserialize(MessageBuffer& buffer, sim::rpc::FunctionCallResponse& out);
void Deserialize(MessageBuffer& buffer, service::ParticipantDiscoveryEvent& out);
void Deserialize(MessageBuffer& buffer, service::ServiceDiscoveryEvent& out);

// Serializers for complete network packets (including size,type kind and service index members)

void Serialize(MessageBuffer& buffer,const logging::LogMsg& msg);
void Serialize(MessageBuffer& buffer,const sync::ParticipantCommand& msg);
void Serialize(MessageBuffer& buffer,const sync::SystemCommand& msg);
void Serialize(MessageBuffer& buffer,const sync::ParticipantStatus& msg);
void Serialize(MessageBuffer& buffer,const sync::ExpectedParticipants& msg);
void Serialize(MessageBuffer& buffer,const sync::NextSimTask& msg);
void Serialize(MessageBuffer& buffer,const sim::data::DataMessageEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::rpc::FunctionCall& msg);
void Serialize(MessageBuffer& buffer,const sim::rpc::FunctionCallResponse& msg);
void Serialize(MessageBuffer& buffer,const service::ParticipantDiscoveryEvent& msg);
void Serialize(MessageBuffer& buffer,const service::ServiceDiscoveryEvent& msg);

}//mw
}//ib
