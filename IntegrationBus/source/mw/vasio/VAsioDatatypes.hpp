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
    uint16_t versionHigh = 3; // If versionHigh/Low changes here, update VIB version range in RegistryMsgHeaderToMainVersionRange
    uint16_t versionLow = 0;
};

struct VAsioMsgSubscriber
{
    EndpointId receiverIdx;
    std::string networkName;
    std::string msgTypeName;
    uint32_t version{0};
};

struct SubscriptionAcknowledge
{
    enum class Status : uint8_t {
        Failed = 0,
        Success = 1
    };
    Status status;
    VAsioMsgSubscriber subscriber;
};

struct ParticipantAnnouncement
{
    RegistryMsgHeader messageHeader;
    ib::mw::VAsioPeerInfo peerInfo;
    //!< additional field as of VIB >3.4.1, will be ignored by older participants
    VAsioPeerUri peerUri;
};

struct ParticipantAnnouncementReply
{
    std::vector<VAsioMsgSubscriber> subscribers;
};

struct KnownParticipants
{
    RegistryMsgHeader messageHeader;
    std::vector<ib::mw::VAsioPeerInfo> peerInfos;
    //!< additional field as of VIB >3.4.1, will be ignored by older participants
    std::vector<VAsioPeerUri> peerUris;
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
//  Inline Implementations
// ================================================================================
inline bool operator!=(const RegistryMsgHeader& lhs, const RegistryMsgHeader& rhs)
{
    return lhs.preambel != rhs.preambel
        || lhs.versionHigh != rhs.versionHigh
        || lhs.versionLow != rhs.versionLow;
}

inline bool operator==(const VAsioMsgSubscriber& lhs, const VAsioMsgSubscriber& rhs)
{
    return lhs.receiverIdx == rhs.receiverIdx 
        && lhs.networkName == rhs.networkName
        && lhs.msgTypeName == rhs.msgTypeName;
}

} // namespace mw
} // namespace ib
