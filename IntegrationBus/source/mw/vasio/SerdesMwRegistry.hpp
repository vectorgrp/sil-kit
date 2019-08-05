// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "RegistryDatatypes.hpp"
#include "VAsioPeerInfo.hpp"

#include "MessageBuffer.hpp"

namespace ib {
namespace mw {

inline MessageBuffer& operator<<(MessageBuffer& buffer, const registry::MessageHeader& header)
{
    buffer << header.preambel
           << header.versionHigh
           << header.versionLow;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::MessageHeader& header)
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
           << peerInfo.acceptorPort;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorHost
           >> peerInfo.acceptorPort;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const registry::ParticipantAnnouncement& announcement)
{
    buffer << announcement.messageHeader
           << announcement.peerInfo;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::ParticipantAnnouncement& announcement)
{
    buffer >> announcement.messageHeader
           >> announcement.peerInfo;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const registry::KnownParticipants& participants)
{
    buffer << participants.messageHeader
           << participants.peerInfos;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::KnownParticipants& participants)
{
    buffer >> participants.messageHeader
           >> participants.peerInfos;
    return buffer;
}

} // namespace mw
} // namespace ib
