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
auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer;
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out);
//! Handshake: Serialize ParticipantAnnouncement (contains peer's protocol version)
auto Serialize(ProtocolVersion version, const ParticipantAnnouncementReply& reply) -> MessageBuffer;
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out);
//! Serialize a Service Subscription request
auto Serialize(const VAsioMsgSubscriber& subscriber) -> MessageBuffer;
void Deserialize(MessageBuffer&, VAsioMsgSubscriber&);
//! Serialize a Service subscription acknowledgement
auto Serialize(ProtocolVersion protocolVersion, const SubscriptionAcknowledge& ack) ->  MessageBuffer;
void Deserialize(MessageBuffer&, SubscriptionAcknowledge&);

auto Serialize(ProtocolVersion protocolVersion ,const KnownParticipants& reply) -> MessageBuffer;
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
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameEvent& out);
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameTransmitEvent& out);
void Deserialize(MessageBuffer& buffer, sim::can::CanControllerStatus& out);
void Deserialize(MessageBuffer& buffer, sim::can::CanConfigureBaudrate& out);
void Deserialize(MessageBuffer& buffer, sim::can::CanSetControllerMode& out);
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

auto Serialize( const logging::LogMsg& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sync::ParticipantCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sync::SystemCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sync::ParticipantStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sync::ExpectedParticipants& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sync::NextSimTask& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::data::DataMessageEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::rpc::FunctionCall& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::rpc::FunctionCallResponse& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::can::CanFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::can::CanFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::can::CanControllerStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::can::CanConfigureBaudrate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::can::CanSetControllerMode& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::eth::EthernetFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::eth::EthernetFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::eth::EthernetStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::eth::EthernetSetMode& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinSendFrameRequest& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinSendFrameHeaderRequest& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinTransmission& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinWakeupPulse& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinControllerConfig& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinControllerStatusUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::lin::LinFrameResponseUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexraySymbolEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexraySymbolTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayCycleStartEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayHostCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayControllerConfig& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayTxBufferConfigUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayTxBufferUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const sim::fr::FlexrayPocStatusEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const service::ParticipantDiscoveryEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;
auto Serialize( const service::ServiceDiscoveryEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer;

}//mw
}//ib
