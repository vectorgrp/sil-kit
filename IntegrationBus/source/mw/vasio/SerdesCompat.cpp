#include "SerdesCompat.hpp"
#include "ib/mw/ParticipantId.hpp"
#include "SerdesMwVAsio.hpp"

namespace version_v3_0
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

} //version_v3_0

namespace ib {
namespace mw {
void DeserializeCompat(ib::mw::MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        version_v3_0::ParticipantAnnouncement oldAnnouncement;
        buffer >> oldAnnouncement;
        announcement.messageHeader = oldAnnouncement.messageHeader;
        auto& info = announcement.peerInfo;
        auto& oldUri = oldAnnouncement.peerUri;
        info.participantName = oldUri.participantName;
        info.participantId = oldUri.participantId;
        info.acceptorUris = oldUri.acceptorUris;
    }
}
void SerializeCompat(MessageBuffer&, const ParticipantAnnouncement&)
{
}
void SerializeCompat(MessageBuffer& buffer, const KnownParticipants& knownParticipants)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        // the VAsioPeerInfo/PeerUri changed, and as such the vector in KnownParticipants
        version_v3_0::KnownParticipants oldAnnouncement;
        oldAnnouncement.messageHeader.versionHigh = 3;
        oldAnnouncement.messageHeader.versionLow = 0;

        for(auto newPeerInfo: knownParticipants.peerInfos)
        {
            //we only copy the peer Uris, the peerInfo 'acceptorHost' and 'acceptorPort'
            // were only used as fallback if the URIs were not set.
            version_v3_0::VAsioPeerUri oldPeerUri;
            oldPeerUri.acceptorUris = newPeerInfo.acceptorUris;
            oldPeerUri.participantId = newPeerInfo.participantId;
            oldPeerUri.participantName = newPeerInfo.participantName;
            oldAnnouncement.peerUris.emplace_back(std::move(oldPeerUri));
        }
        
        // serialize old
        buffer << oldAnnouncement;
    }
}
void DeserializeCompat(MessageBuffer&, KnownParticipants&)
{
}

void DeserializeCompat(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        version_v3_0::ParticipantAnnouncementReply oldReply;
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
void SerializeCompat(MessageBuffer& buffer, const ParticipantAnnouncementReply& newReply)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        // the ParticipantAnnouncementReply was extended for proto v3.1
        version_v3_0::ParticipantAnnouncementReply oldReply;
        for(const auto& subscriber: newReply.subscribers)
        {
            oldReply.subscribers.push_back(subscriber);
        }
        buffer << oldReply;
    }
}

} //mw
} //ib
