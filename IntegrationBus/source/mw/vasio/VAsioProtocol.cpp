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
#include "SerdesMw.hpp"
#include "SerdesMwVAsio.hpp"

#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesSimData.hpp"
#include "SerdesSimRpc.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimLin.hpp"
#include "SerdesSimFlexray.hpp"
#include "SerdesMwService.hpp"


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

//Pack complete network message
template<typename MessageT>
auto Pack(const MessageT& msg, const EndpointAddress& endpointAddress,
    EndpointId remoteIdx) -> MessageBuffer
{
    ib::mw::MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};
    buffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbSimMsg
        << remoteIdx
        << endpointAddress
        << msg;
    return buffer;
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
auto Serialize(const ParticipantAnnouncementReply& reply) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncementReply
           << reply;
    return buffer;
}
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncement
           << announcement;
    return buffer;
}
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}


 auto Serialize(const VAsioMsgSubscriber& subscriber) -> MessageBuffer
{
    ib::mw::MessageBuffer buffer;
    uint32_t rawMsgSize{0};
    buffer
        << rawMsgSize
        << VAsioMsgKind::SubscriptionAnnouncement
        << subscriber;

    return buffer;
}
void Deserialize(MessageBuffer& buffer, VAsioMsgSubscriber& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

// Subscription of services have a negotiated "connection"-version
 auto Serialize(uint32_t protocolVersion, const SubscriptionAcknowledge& ack) ->  MessageBuffer
{
    MessageBuffer ackBuffer;
    ackBuffer.SetFormatVersion(protocolVersion);

    uint32_t msgSizePlaceholder{0u};
    ackBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::SubscriptionAcknowledge
        << ack;
    return ackBuffer;
}
void Deserialize(MessageBuffer& buffer, SubscriptionAcknowledge& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

auto Serialize(const KnownParticipants& knownParticipantsMsg) -> MessageBuffer
{
    MessageBuffer sendBuffer;
    uint32_t msgSizePlaceholder{0u};
    sendBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbRegistryMessage
        << RegistryMessageKind::KnownParticipants
        << knownParticipantsMsg;
    return sendBuffer;
}
void Deserialize(MessageBuffer& buffer,KnownParticipants& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}


//////////////////////////////////////////////////////////////////////
//  Protocol Versioning
//////////////////////////////////////////////////////////////////////
 bool ProtocolVersionSupported(const ParticipantAnnouncement& /*announcement*/)
{
    return true;
}

 auto ProtocolVersionToString(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string
{
    if (registryMsgHeader.versionHigh == 1)
    {
        return {"< v2.0.0"};
    }
    else if (registryMsgHeader.versionHigh == 2 && registryMsgHeader.versionLow == 0)
    {
        return {"v2.0.0 - v3.4.0"};
    }
    else if (registryMsgHeader.versionHigh == 2 && registryMsgHeader.versionLow == 1)
    {
        return {"v3.4.1 - v3.99.21"};
    }
    else if (registryMsgHeader.versionHigh == 3 && registryMsgHeader.versionLow == 0)
    {
        return {"v3.99.22 - current"};
    }

    return {"Unknown version range"};
}

////////////////////////////////////////////////////////////////////////////////
// Services for established connections
////////////////////////////////////////////////////////////////////////////////

void Deserialize(MessageBuffer& buffer, mw::logging::LogMsg& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::sync::ParticipantCommand& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::sync::SystemCommand& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::sync::ParticipantStatus& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::sync::ExpectedParticipants& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::sync::NextSimTask& out)
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
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::can::CanFrameTransmitEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::can::CanControllerStatus& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::can::CanConfigureBaudrate& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::can::CanSetControllerMode& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetFrameEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetFrameTransmitEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetStatus& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::eth::EthernetSetMode& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinSendFrameRequest& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinSendFrameHeaderRequest& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinTransmission& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinWakeupPulse& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinControllerConfig& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinControllerStatusUpdate& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::lin::LinFrameResponseUpdate& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayFrameEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayFrameTransmitEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexraySymbolEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexraySymbolTransmitEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayCycleStartEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayHostCommand& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayControllerConfig& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayTxBufferConfigUpdate& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayTxBufferUpdate& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, sim::fr::FlexrayPocStatusEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::service::ParticipantDiscoveryEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}
void Deserialize(MessageBuffer& buffer, mw::service::ServiceDiscoveryEvent& out)
{
    out = Unpack<std::remove_reference_t<decltype(out)>>(buffer);
}

//////////////////////////////////////////////////////////////////////
// Serializers
//////////////////////////////////////////////////////////////////////
auto Serialize( const logging::LogMsg& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sync::ParticipantCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sync::SystemCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sync::ParticipantStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sync::ExpectedParticipants& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sync::NextSimTask& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sim::data::DataMessageEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::rpc::FunctionCall& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::rpc::FunctionCallResponse& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::can::CanFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::can::CanFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::can::CanControllerStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::can::CanConfigureBaudrate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::can::CanSetControllerMode& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);

}
auto Serialize( const sim::eth::EthernetFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::eth::EthernetFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::eth::EthernetStatus& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::eth::EthernetSetMode& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinSendFrameRequest& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinSendFrameHeaderRequest& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinTransmission& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinWakeupPulse& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinControllerConfig& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinControllerStatusUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::lin::LinFrameResponseUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayFrameEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayFrameTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexraySymbolEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexraySymbolTransmitEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayCycleStartEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayHostCommand& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayControllerConfig& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayTxBufferConfigUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayTxBufferUpdate& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const sim::fr::FlexrayPocStatusEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const service::ParticipantDiscoveryEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}
auto Serialize( const service::ServiceDiscoveryEvent& msg, const EndpointAddress& epa, EndpointId remoteIndex)-> MessageBuffer
{
    return Pack(msg, epa, remoteIndex);
}

}//mw
}//ib
