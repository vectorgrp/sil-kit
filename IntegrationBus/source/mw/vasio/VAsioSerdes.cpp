// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioSerdes.hpp"
#include "VAsioPeerInfo.hpp"

#include "Uri.hpp"
#include "InternalSerdes.hpp"
#include "ProtocolVersion.hpp"



namespace ib {
namespace mw {

// required for protocol_3_0:: and current 
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

// Backward compatible serdes with ProtocolVersion{3,0}
namespace protocol_3_0
{
//these two structures were merged for v3.1
struct VAsioPeerInfo
{
    std::string participantName;
    ib::mw::ParticipantId participantId;
    std::string acceptorHost;
    uint16_t acceptorPort;
};

struct VAsioPeerUri
{
    std::string participantName;
    ib::mw::ParticipantId participantId;
    std::vector<std::string> acceptorUris;
};

struct ParticipantAnnouncement
{
    ib::mw::RegistryMsgHeader messageHeader; //not changed
    VAsioPeerInfo peerInfo;
    //!< additional field as of VIB >3.4.1, will be ignored by older participants
    VAsioPeerUri peerUri;
};

struct ParticipantAnnouncementReply
{
    std::vector<ib::mw::VAsioMsgSubscriber> subscribers;
};

struct KnownParticipants
{
    ib::mw::RegistryMsgHeader messageHeader;
    std::vector<VAsioPeerInfo> peerInfos;
    //!< additional field as of VIB >3.4.1, will be ignored by older participants
    std::vector<VAsioPeerUri> peerUris;
};

// Actual Serdes Code
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const VAsioPeerInfo& peerInfo)
{
    buffer << peerInfo.participantName
           << peerInfo.participantId
           << peerInfo.acceptorHost
           << peerInfo.acceptorPort;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorHost
           >> peerInfo.acceptorPort;
    return buffer;
}

//SerDes v3.1
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const VAsioPeerUri& peerUri)
{
    buffer << peerUri.participantName
           << peerUri.participantId
           << peerUri.acceptorUris
           ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, VAsioPeerUri& peerUri)
{
    buffer >> peerUri.participantName
           >> peerUri.participantId
           >> peerUri.acceptorUris
        ;
    return buffer;
}


inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    buffer << announcement.messageHeader
        << announcement.peerInfo
        << announcement.peerUri
        ;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    buffer >> announcement.messageHeader
        >> announcement.peerInfo
        >> announcement.peerUri
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const KnownParticipants& participants)
{
    buffer << participants.messageHeader
        << participants.peerInfos
        << participants.peerUris;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, KnownParticipants& participants)
{
    buffer >> participants.messageHeader
        >> participants.peerInfos;
    if (buffer.RemainingBytesLeft() > 0)
    {
        buffer >> participants.peerUris;
    }
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    buffer << reply.subscribers;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    buffer >> reply.subscribers;
    return buffer;
}

} //protocol_3_0


// Helper  for ProtocolVersion{3,0}
void DeserializeCompat(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        protocol_3_0::ParticipantAnnouncementReply oldReply;
        reply.remoteHeader.versionHigh = 3;
        reply.remoteHeader.versionHigh = 0;
        // Status was not part of  < v3.1
        reply.status = ParticipantAnnouncementReply::Status::Success;
        // subscribers is the same
        buffer >> oldReply;
        for(const auto& subscriber: oldReply.subscribers)
        {
            reply.subscribers.push_back(subscriber);
        }
    }
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

    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        protocol_3_0::ParticipantAnnouncement oldAnnouncement{};
        auto& oldUri = oldAnnouncement.peerUri;
        oldUri.participantName = announcement.peerInfo.participantName;
        oldUri.participantId = announcement.peerInfo.participantId;
        oldUri.acceptorUris = announcement.peerInfo.acceptorUris;
        buffer << oldAnnouncement;
    }
    else if (buffer.GetFormatVersion() == CurrentProtocolVersion())
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
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        protocol_3_0::ParticipantAnnouncement oldAnnouncement;
        buffer >> oldAnnouncement;
        announcement.messageHeader = oldAnnouncement.messageHeader;
        auto& info = announcement.peerInfo;
        auto& oldUri = oldAnnouncement.peerUri;
        info.participantName = oldUri.participantName;
        info.participantId = oldUri.participantId;
        info.acceptorUris = oldUri.acceptorUris;
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
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        // the ParticipantAnnouncementReply was extended for proto v3.1
        protocol_3_0::ParticipantAnnouncementReply oldReply;
        for(const auto& subscriber: reply.subscribers)
        {
            oldReply.subscribers.push_back(subscriber);
        }
        buffer << oldReply;
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
    if (buffer.GetFormatVersion() != CurrentProtocolVersion())
    {
        // Backward compatibility here is tricky. When connecting to a VAsioRegistry
        // we already negotiated a ProtocolVersion via the KnownParticipants message.
        // In all other cases we do not know the ProtocolVersion a priori here
        if (buffer.GetFormatVersion() == ProtocolVersion{0,0})
        {
            //Ok, uninitialized ProtocolVersion implies that we have a connection between two, non-registry peers
            //Let's guess the version based on the buffer size
            auto bufferCopy = buffer;
            try {
                // Try the current version, this contains a remoteHeader field which should be more future proof
                ParticipantAnnouncementReply maybeReply;
                bufferCopy 
                    >> maybeReply.remoteHeader
                    >> maybeReply.status
                    >> maybeReply.subscribers
                    ;
                reply = maybeReply;
            } catch(...) {
                //fall through to the backward compatible code
                buffer.SetFormatVersion({3,0});
                DeserializeCompat(buffer, reply);
            }
        }
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
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        // the VAsioPeerInfo/PeerUri changed, and as such the vector in KnownParticipants
        protocol_3_0::KnownParticipants oldAnnouncement;
        oldAnnouncement.messageHeader.versionHigh = 3;
        oldAnnouncement.messageHeader.versionLow = 0;

        for(auto newPeerInfo: participants.peerInfos)
        {
            //we only copy the peer Uris, the peerInfo 'acceptorHost' and 'acceptorPort'
            // were only used as fallback if the URIs were not set.
            protocol_3_0::VAsioPeerUri oldPeerUri;
            oldPeerUri.acceptorUris = newPeerInfo.acceptorUris;
            oldPeerUri.participantId = newPeerInfo.participantId;
            oldPeerUri.participantName = newPeerInfo.participantName;
            oldAnnouncement.peerUris.emplace_back(std::move(oldPeerUri));
        }
        
        // serialize old
        buffer << oldAnnouncement;
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
    buffer >> participants.messageHeader
        >> participants.peerInfos
        ;
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

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncementReply& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncement& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, VAsioMsgSubscriber& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const SubscriptionAcknowledge& msg)
{
    buffer<< msg;
    return;
}
void Deserialize(MessageBuffer& buffer, SubscriptionAcknowledge& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const KnownParticipants& msg)
{
    buffer << msg;
    return;
}
void Deserialize(MessageBuffer& buffer,KnownParticipants& out)
{
    buffer >> out;
}

} // namespace mw
} // namespace ib
