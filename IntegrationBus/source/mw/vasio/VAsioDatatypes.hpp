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
    uint16_t versionHigh = 1;
    uint16_t versionLow = 0;
};

struct ParticipantAnnouncement
{
    RegistryMsgHeader messageHeader;
    ib::mw::VAsioPeerInfo peerInfo;
};

struct KnownParticipants
{
    RegistryMsgHeader messageHeader;
    std::vector<ib::mw::VAsioPeerInfo> peerInfos;
};

enum class RegistryMessageKind : uint8_t
{
    Invalid = 0,
    ParticipantAnnouncement = 1,
    KnownParticipants = 2,
    SubscriptionSent = 3
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
