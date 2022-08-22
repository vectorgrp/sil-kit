/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <string>
#include <cstdint>
#include <vector>
#include <array>

#include "VAsioSerdes_Protocol30.hpp"

namespace protocol_3_0 {

using SilKit::Core::MessageBuffer;
// Backward compatible serdes with ProtocolVersion{3,0}

// header is fixed at this version
struct RegistryMsgHeader
{
    std::array<uint8_t, 4> preambel{{'V', 'I', 'B', '-'}};
    uint16_t versionHigh{3};
    uint16_t versionLow{0};
};

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

// the older VAsioMsgSubscriber lacks the version field
struct VAsioMsgSubscriber
{
    SilKit::Core::EndpointId receiverIdx;
    std::string networkName;
    std::string msgTypeName;
};

inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber)
{
    buffer << subscriber.receiverIdx
           << subscriber.networkName
           << subscriber.msgTypeName
        ;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioMsgSubscriber& subscriber)
{
    buffer >> subscriber.receiverIdx
           >> subscriber.networkName
           >> subscriber.msgTypeName
        ;
    return buffer;
}

// these two structures were merged for v3.1
struct VAsioPeerInfo
{
    std::string participantName;
    SilKit::Core::ParticipantId participantId;
    std::string acceptorHost;
    uint16_t acceptorPort;
};

struct VAsioPeerUri
{
    std::string participantName;
    SilKit::Core::ParticipantId participantId;
    std::vector<std::string> acceptorUris;
};

struct ParticipantAnnouncement
{
    RegistryMsgHeader messageHeader; //not changed
    VAsioPeerInfo peerInfo;
    //!< additional field as of SILKIT >3.4.1, will be ignored by older participants
    VAsioPeerUri peerUri;
};

struct ParticipantAnnouncementReply
{
    std::vector<VAsioMsgSubscriber> subscribers;
};

struct KnownParticipants
{
    RegistryMsgHeader messageHeader;
    std::vector<VAsioPeerInfo> peerInfos;
    //!< additional field as of SILKIT >3.4.1, will be ignored by older participants
    std::vector<VAsioPeerUri> peerUris;
};

// Actual Serdes Code
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const VAsioPeerInfo& peerInfo)
{
    buffer << peerInfo.participantName
           << peerInfo.participantId
           << peerInfo.acceptorHost
           << peerInfo.acceptorPort;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorHost
           >> peerInfo.acceptorPort;
    return buffer;
}

//SerDes v3.1
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const VAsioPeerUri& peerUri)
{
    buffer << peerUri.participantName
           << peerUri.participantId
           << peerUri.acceptorUris
           ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, VAsioPeerUri& peerUri)
{
    buffer >> peerUri.participantName
           >> peerUri.participantId
           >> peerUri.acceptorUris
        ;
    return buffer;
}


inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    buffer << announcement.messageHeader
        << announcement.peerInfo
        << announcement.peerUri
        ;

    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    buffer >> announcement.messageHeader
        >> announcement.peerInfo
        >> announcement.peerUri
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const KnownParticipants& participants)
{
    buffer << participants.messageHeader
        << participants.peerInfos
        << participants.peerUris;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, KnownParticipants& participants)
{
    buffer >> participants.messageHeader
        >> participants.peerInfos;
    if (buffer.RemainingBytesLeft() > 0)
    {
        buffer >> participants.peerUris;
    }
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    buffer << reply.subscribers;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    buffer >> reply.subscribers;
    return buffer;
}

} // namespace protocol_3_0

namespace SilKit {
namespace Core {

void DeserializeV30(MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    //need legacy support here, convert old format to current one
    protocol_3_0::ParticipantAnnouncement oldAnnouncement;
    buffer >> oldAnnouncement;
    announcement.messageHeader.versionHigh = oldAnnouncement.messageHeader.versionHigh;
    announcement.messageHeader.versionLow = oldAnnouncement.messageHeader.versionLow;
    auto& info = announcement.peerInfo;
    auto& oldUri = oldAnnouncement.peerUri;
    info.participantName = oldUri.participantName;
    info.participantId = oldUri.participantId;
    info.acceptorUris = oldUri.acceptorUris;
}

void SerializeV30(MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    //need legacy support here, convert old format to current one
    protocol_3_0::ParticipantAnnouncement oldAnnouncement{};
    auto& oldUri = oldAnnouncement.peerUri;
    oldUri.participantName = announcement.peerInfo.participantName;
    oldUri.participantId = announcement.peerInfo.participantId;
    oldUri.acceptorUris = announcement.peerInfo.acceptorUris;
    buffer << oldAnnouncement;
}

void DeserializeV30(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    // need legacy support here, convert old format to current one
    protocol_3_0::ParticipantAnnouncementReply oldReply;
    reply.remoteHeader.versionHigh = 3;
    reply.remoteHeader.versionHigh = 0;
    // Status was not part of  < v3.1
    reply.status = ParticipantAnnouncementReply::Status::Success;
    // subscribers is the same
    buffer >> oldReply;
    for (const auto& subscriber : oldReply.subscribers)
    {
        VAsioMsgSubscriber newSubscriber;
        newSubscriber.msgTypeName = subscriber.msgTypeName;
        newSubscriber.networkName = subscriber.networkName;
        newSubscriber.receiverIdx = subscriber.receiverIdx;
        newSubscriber.version = 0; // not part of the legacy message
        reply.subscribers.emplace_back(std::move(newSubscriber));
    }
}

void SerializeV30(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    // the ParticipantAnnouncementReply was extended for proto v3.1
    protocol_3_0::ParticipantAnnouncementReply oldReply;
    for (const auto& subscriber : reply.subscribers)
    {

        protocol_3_0::VAsioMsgSubscriber oldSubscriber;
        oldSubscriber.msgTypeName = subscriber.msgTypeName;
        oldSubscriber.networkName = subscriber.networkName;
        oldSubscriber.receiverIdx = subscriber.receiverIdx;
        oldReply.subscribers.emplace_back(std::move(oldSubscriber));
    }
    buffer << oldReply;
}

void DeserializeV30(MessageBuffer& buffer, KnownParticipants& participants)
{
    protocol_3_0::KnownParticipants oldParticipants;
    buffer >> oldParticipants;
    for (const auto& peerUri : oldParticipants.peerUris)
    {
        SilKit::Core::VAsioPeerInfo info;
        info.participantName = peerUri.participantName;
        info.participantId = peerUri.participantId;
        info.acceptorUris = peerUri.acceptorUris;
        participants.peerInfos.emplace_back(std::move(info));
    }
}

void SerializeV30(MessageBuffer& buffer, const KnownParticipants& participants)
{
    // the VAsioPeerInfo/PeerUri changed, and as such the vector in KnownParticipants
    protocol_3_0::KnownParticipants oldParticipants;
    oldParticipants.messageHeader.versionHigh = 3;
    oldParticipants.messageHeader.versionLow = 0;

    for (const auto& newPeerInfo : participants.peerInfos)
    {
        // we only copy the peer Uris, the peerInfo 'acceptorHost' and 'acceptorPort'
        // were only used as fallback if the URIs were not set.
        protocol_3_0::VAsioPeerUri oldPeerUri;
        oldPeerUri.acceptorUris = newPeerInfo.acceptorUris;
        oldPeerUri.participantId = newPeerInfo.participantId;
        oldPeerUri.participantName = newPeerInfo.participantName;
        oldParticipants.peerUris.emplace_back(std::move(oldPeerUri));
    }

    // serialize old
    buffer << oldParticipants;
}

} // namespace Core
} // namespace SilKit
