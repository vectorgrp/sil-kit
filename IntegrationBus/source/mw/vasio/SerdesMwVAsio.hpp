// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "VAsioDatatypes.hpp"
#include "VAsioPeerInfo.hpp"

#include "MessageBuffer.hpp"

namespace ib {
namespace mw {

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
           << peerInfo.acceptorHost
           << peerInfo.acceptorPort
           << peerInfo.acceptorUris
        ;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorHost
           >> peerInfo.acceptorPort;
    // In version <= 3.4.0 there was no acceptorUris, we optionally
    // deserialize it for backwards compatibility with older clients.
    try
    {
        buffer >> peerInfo.acceptorUris;
    }
    catch (...)
    {
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber)
{
    buffer << subscriber.receiverIdx
           << subscriber.linkName
           << subscriber.msgTypeName;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioMsgSubscriber& subscriber)
{
    buffer >> subscriber.receiverIdx
           >> subscriber.linkName
           >> subscriber.msgTypeName;
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
    buffer << announcement.messageHeader
           << announcement.peerInfo;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    buffer >> announcement.messageHeader
           >> announcement.peerInfo;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    buffer << reply.subscribers;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    buffer >> reply.subscribers;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const KnownParticipants& participants)
{
    buffer << participants.messageHeader
           << participants.peerInfos;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, KnownParticipants& participants)
{
    buffer >> participants.messageHeader
           >> participants.peerInfos;
    return buffer;
}

} // namespace mw
} // namespace ib
