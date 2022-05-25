// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "VAsioProtocol.hpp"
#include <type_traits>

// Simulation IB DataTypes

#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/eth/EthernetDatatypes.hpp"
#include "ib/sim/fr/FlexrayDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"
#include "ib/mw/sync/SyncDatatypes.hpp"

// message buffer Serdes
#include "SerdesMwVAsio.hpp"

#include "SerdesMwLogging.hpp"
#include "SerdesSimData.hpp"
#include "SerdesSimRpc.hpp"

#include "InternalSerdes.hpp"

namespace {
// Unpack messages which has alreayd unpacked header members
using namespace ib::mw;
template<typename MessageT>
auto Unpack(MessageBuffer& buffer) -> MessageT
{
    MessageT value;
    buffer >> value;
    return value;
}
}
namespace ib {
namespace mw {
// Handshake primitives for wire format
auto ExtractMessageSize(MessageBuffer& buffer) -> uint32_t
{
    uint32_t messageSize{0};
    buffer >> messageSize;
    return messageSize;
}
// Extract the message kind tag (second element in wire format message)
auto ExtractMessageKind(MessageBuffer& buffer) -> VAsioMsgKind
{
    VAsioMsgKind messageKind{};
    buffer >> messageKind;
    return messageKind;
}

auto ExtractRegistryMessageKind(MessageBuffer& buffer) -> RegistryMessageKind
{
    RegistryMessageKind kind;
    buffer >> kind;
    return kind;
}

auto PeekRegistryMessageHeader(const MessageBuffer& buffer) -> RegistryMsgHeader
{
    // we do not want change buffer's internal state, so we copy it, since we do not have dedicated peek methods on it
    auto bufferCopy = buffer;
    RegistryMsgHeader header;
    bufferCopy >> header;
    return header;
}

auto ExtractEndpointId(MessageBuffer& buffer) ->EndpointId
{
    EndpointId endpointId;
    buffer >> endpointId;
    return endpointId;
}
auto ExtractEndpointAddress(MessageBuffer& buffer) ->EndpointAddress
{
    EndpointAddress endpointAddress;
    buffer >> endpointAddress;
    return endpointAddress;
}
//////////////////////////////////////////////////////////////////////
//     Serialize
//////////////////////////////////////////////////////////////////////
void Serialize(MessageBuffer buffer, const ParticipantAnnouncementReply& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncement& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

 void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, VAsioMsgSubscriber& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

 void Serialize(MessageBuffer buffer, const SubscriptionAcknowledge& msg)
{
    buffer<< msg;
    return;
}
void Deserialize(MessageBuffer& buffer, SubscriptionAcknowledge& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

void Serialize(MessageBuffer& buffer, const KnownParticipants& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer,KnownParticipants& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}


//////////////////////////////////////////////////////////////////////
//  Protocol Versioning
//////////////////////////////////////////////////////////////////////
bool ProtocolVersionSupported(const RegistryMsgHeader& header)
{
    const auto version = from_header(header);
    if(version == ProtocolVersion{3, 0})
    {
        //3.99.21: bumped version to be explicitly incompatible with prior releases (MVP3, CANoe16)
        return true;
    }
    else if (version == ProtocolVersion{3,1})
    {
        //3.99.23: bumped version to test backwards compatibility with removed VAsioPeerUri in ParticipantAnnouncement
        return true;
    }
    // NB: Add your explicit backward compatibility here, ensure that Serialize/Deserialize can handle the ProtocolVersion transparently.

    return false;
 }


////////////////////////////////////////////////////////////////////////////////
// Services for established connections
////////////////////////////////////////////////////////////////////////////////

void Deserialize(MessageBuffer& buffer, mw::logging::LogMsg& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::data::DataMessageEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::rpc::FunctionCall& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::rpc::FunctionCallResponse& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

//////////////////////////////////////////////////////////////////////
// Serializers
//////////////////////////////////////////////////////////////////////
void Serialize(MessageBuffer& buffer, const logging::LogMsg& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::data::DataMessageEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::rpc::FunctionCall& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const sim::rpc::FunctionCallResponse& msg)
{
    buffer << msg;
    return;
}

} // namespace mw
} // namespace ib
