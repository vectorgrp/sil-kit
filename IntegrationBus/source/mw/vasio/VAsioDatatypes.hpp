// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <array>

#include "VAsioPeerInfo.hpp"

namespace ib {
namespace mw {

struct RegistryMsgHeader
{
    std::array<char, 4> preambel{{'V', 'I', 'B', '-'}};
    uint16_t versionHigh = 2;
    uint16_t versionLow = 0;
};

struct VAsioMsgSubscriber
{
    uint16_t    receiverIdx;
    std::string linkName;
    std::string msgTypeName;
};

struct ParticipantAnnouncement
{
    RegistryMsgHeader messageHeader;
    ib::mw::VAsioPeerInfo peerInfo;
};

struct ParticipantAnnouncementReply
{
    std::vector<VAsioMsgSubscriber> subscribers;
};

struct KnownParticipants
{
    RegistryMsgHeader messageHeader;
    std::vector<ib::mw::VAsioPeerInfo> peerInfos;
};

enum class RegistryMessageKind : uint8_t
{
    Invalid = 0,
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !! DO NOT CHANGE THE VALUE OF ParticipantAnnouncement !!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // The ParticipantAnnouncement is the first message transmitted over a new
    // connection and carries the protocol version. Thus, changing the enum
    // value of ParticipantAnnouncement will break protocol break detections
    // with older participants.
    ParticipantAnnouncement = 1,
    ParticipantAnnouncementReply = 2,
    KnownParticipants = 3
};


// ================================================================================
//  Inline Implementaitons
// ================================================================================
inline bool operator!=(const RegistryMsgHeader& lhs, const RegistryMsgHeader& rhs)
{
    return lhs.preambel != rhs.preambel
        || lhs.versionHigh != rhs.versionHigh
        || lhs.versionLow != rhs.versionLow;
}



} // namespace mw
} // namespace ib
