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
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetFrameEvent& out);
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetFrameTransmitEvent& out);
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetStatus& out);
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetSetMode& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinSendFrameRequest& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinSendFrameHeaderRequest& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinTransmission& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinWakeupPulse& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinControllerConfig& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinControllerStatusUpdate& out);
void Deserialize(MessageBuffer& buffer, sim::lin::LinFrameResponseUpdate& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayFrameEvent& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayFrameTransmitEvent& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexraySymbolEvent& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexraySymbolTransmitEvent& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayCycleStartEvent& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayHostCommand& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayControllerConfig& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayTxBufferConfigUpdate& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayTxBufferUpdate& out);
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayPocStatusEvent& out);
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
void Serialize(MessageBuffer& buffer,const sim::eth::EthernetFrameEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::eth::EthernetFrameTransmitEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::eth::EthernetStatus& msg);
void Serialize(MessageBuffer& buffer,const sim::eth::EthernetSetMode& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinSendFrameRequest& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinSendFrameHeaderRequest& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinTransmission& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinWakeupPulse& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinControllerConfig& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinControllerStatusUpdate& msg);
void Serialize(MessageBuffer& buffer,const sim::lin::LinFrameResponseUpdate& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayFrameEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayFrameTransmitEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexraySymbolEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexraySymbolTransmitEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayCycleStartEvent& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayHostCommand& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayControllerConfig& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayTxBufferConfigUpdate& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayTxBufferUpdate& msg);
void Serialize(MessageBuffer& buffer,const sim::fr::FlexrayPocStatusEvent& msg);
void Serialize(MessageBuffer& buffer,const service::ParticipantDiscoveryEvent& msg);
void Serialize(MessageBuffer& buffer,const service::ServiceDiscoveryEvent& msg);

}//mw
}//ib
