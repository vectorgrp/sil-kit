// Copyright (c) Vector Informatik GmbH. All rights reserved.
// VAsio network protocol
// Use the Deserialize/Serialize functions for versioned
// (backward compatible) access to the protocol wire format

#pragma once
#include "VAsioMsgKind.hpp"
#include "MessageBuffer.hpp"
#include "SerdesMw.hpp"
#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesMwVAsio.hpp"
#include "SerdesSimData.hpp"
#include "SerdesSimRpc.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimLin.hpp"
#include "SerdesSimFlexray.hpp"
#include "SerdesMwService.hpp"

namespace ib {
namespace mw {
// SerDes helpers to reduce boiler plate
// Early handshake does not have a negotiated version, yet:

//! Handshake: Serialize ParticipantAnnouncementReply (contains remote peer's protocol version)
auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer;
//! Handshake: Serialize ParticipantAnnouncement (contains peer's protocol version)
auto Serialize(const ParticipantAnnouncementReply& reply) -> MessageBuffer;
//! Serialize a Service Subscription request
auto Serialize(const VAsioMsgSubscriber& subscriber) -> MessageBuffer;
//! Serialize a Service subscription acknowledgement
auto Serialize(uint32_t protocolVersion, const SubscriptionAcknowledge& ack) ->  MessageBuffer;

//! Check if we support ser/des for the given protocol version
bool ProtocolVersionSupported(const ParticipantAnnouncement& announcement);
//! Map Protocol Versions to VIB releases
auto ProtocolVersionToString(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string;

}//mw
}//ib
