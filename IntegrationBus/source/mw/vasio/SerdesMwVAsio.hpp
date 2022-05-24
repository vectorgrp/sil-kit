// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "VAsioDatatypes.hpp"
#include "VAsioPeerInfo.hpp"

#include "Uri.hpp"
#include "MessageBuffer.hpp"
#include "SerdesCompat.hpp"
#include "VAsioProtocolVersion.hpp"

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
    buffer << announcement.messageHeader
        << announcement.peerInfo
        ;

    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    //Backward compatibility
    const auto currentVersion = CurrentProtocolVersion();
    if (buffer.GetFormatVersion() != currentVersion)
    {
        DeserializeCompat(buffer, announcement);
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
    const auto currentVersion = CurrentProtocolVersion();
    if (buffer.GetFormatVersion() != currentVersion)
    {
        SerializeCompat(buffer, reply);
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
    const auto currentVersion = CurrentProtocolVersion();
    if (buffer.GetFormatVersion() != currentVersion)
    {
        // Backward compatibiliy here is tricky. When connecting to a VAsioRegistry
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
    const auto currentVersion = CurrentProtocolVersion();
    if (buffer.GetFormatVersion() != currentVersion)
    {
        SerializeCompat(buffer, participants);
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


} // namespace mw
} // namespace ib
