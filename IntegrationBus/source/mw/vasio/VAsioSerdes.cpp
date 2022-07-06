// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioSerdes.hpp"
#include "VAsioPeerInfo.hpp"

#include "Uri.hpp"
#include "InternalSerdes.hpp"
#include "ProtocolVersion.hpp"
#include "VAsioProtocolVersion.hpp" // from_header(ProtcolVersion)

// Backward compatibility:
#include "VAsioSerdes_Protocol30.hpp"

namespace SilKit {
namespace Core {

inline MessageBuffer& operator<<(MessageBuffer& buffer, const RegistryMsgHeader& header)
{
    buffer << header.preambel
           << header.versionHigh
           << header.versionLow;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, RegistryMsgHeader& header)
{
    buffer >> header.preambel
           >> header.versionHigh
           >> header.versionLow;
    return buffer;
}


inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioPeerInfo& peerInfo)
{
    buffer << peerInfo.participantName
           << peerInfo.participantId
           << peerInfo.acceptorUris
           << peerInfo.capabilities
           ;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorUris
           >> peerInfo.capabilities
        ;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber)
{
    buffer << subscriber.receiverIdx
           << subscriber.networkName
           << subscriber.msgTypeName
           << subscriber.version
        ;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioMsgSubscriber& subscriber)
{
    buffer >> subscriber.receiverIdx
           >> subscriber.networkName
           >> subscriber.msgTypeName
           >> subscriber.version
        ;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const SubscriptionAcknowledge& ack)
{
    buffer << ack.status
           << ack.subscriber;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, SubscriptionAcknowledge& ack)
{
    buffer >> ack.status
           >> ack.subscriber;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    // ParticipantAnnouncement is the first message sent during a handshake.
    // so we need to extract its version information for ser/des here.
    buffer.SetProtocolVersion(from_header(announcement.messageHeader));
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, announcement);
    }
    else
    {
        buffer
            << announcement.messageHeader
            << announcement.peerInfo
            ;
    }

    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        DeserializeV30(buffer, announcement);
    }
    else
    {
        //  default
        buffer
            >> announcement.messageHeader
            >> announcement.peerInfo
            ;
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, reply);
    }
    else
    {
        buffer  
            << reply.remoteHeader
            << reply.status
            << reply.subscribers;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        buffer.SetProtocolVersion({3,0});
        DeserializeV30(buffer, reply);
    }
    else
    {
        buffer 
            >> reply.remoteHeader
            >> reply.status
            >> reply.subscribers;
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const KnownParticipants& participants)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, participants);
    }
    else
    {
        buffer << participants.messageHeader
            << participants.peerInfos
            ;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, KnownParticipants& participants)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        DeserializeV30(buffer, participants);
    }
    else
    {
        buffer >> participants.messageHeader
            >> participants.peerInfos
            ;
    }
    return buffer;
}

//////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////

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

auto PeekRegistryMessageHeader(MessageBuffer& buffer) -> RegistryMsgHeader
{
    // read only the header into a new MessageBuffer
    auto data = buffer.PeekData();
    const auto readPos = buffer.ReadPos();
    std::vector<uint8_t> rawHeader;
    rawHeader.resize(sizeof(RegistryMsgHeader));
    memcpy(rawHeader.data(), (data.data() + readPos), sizeof(RegistryMsgHeader));

    MessageBuffer headerBuffer(std::move(rawHeader));
    RegistryMsgHeader header;
    headerBuffer >> header;
    return header;
}

auto ExtractEndpointId(MessageBuffer& buffer) -> EndpointId
{
    EndpointId endpointId;
    buffer >> endpointId;
    return endpointId;
}
auto ExtractEndpointAddress(MessageBuffer& buffer) -> EndpointAddress
{
    EndpointAddress endpointAddress;
    buffer >> endpointAddress;
    return endpointAddress;
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncementReply& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncement& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, VAsioMsgSubscriber& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const SubscriptionAcknowledge& msg)
{
    buffer<< msg;
}
void Deserialize(MessageBuffer& buffer, SubscriptionAcknowledge& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const KnownParticipants& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer,KnownParticipants& out)
{
    buffer >> out;
}

} // namespace Core
} // namespace SilKit
