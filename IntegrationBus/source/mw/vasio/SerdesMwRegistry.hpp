// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "../registry/RegistryDatatypes.hpp"
#include "VAsioPeerInfo.hpp"

#include "MessageBuffer.hpp"

namespace ib {
namespace mw {

inline MessageBuffer& operator<<(MessageBuffer& buffer, const registry::RegistryMessageHeader& header)
{
    buffer << header.preambel
           << header.versionHigh
           << header.versionLow;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::RegistryMessageHeader& header)
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
    buffer << dynamic_cast<const registry::RegistryMessageHeader&>(announcement)
           << announcement.localInfo;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::ParticipantAnnouncement& announcement)
{
    buffer >> dynamic_cast<registry::RegistryMessageHeader&>(announcement)
           >> announcement.localInfo;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const registry::KnownParticipants& participants)
{
    buffer << dynamic_cast<const registry::RegistryMessageHeader&>(participants)
           << participants.participantInfos;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, registry::KnownParticipants& participants)
{
    buffer >> dynamic_cast<registry::RegistryMessageHeader&>(participants)
           >> participants.participantInfos;
    return buffer;
}

} // namespace mw
} // namespace ib
