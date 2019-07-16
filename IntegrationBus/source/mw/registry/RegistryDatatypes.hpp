// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <array>

#include "../vasio/VAsioPeerInfo.hpp"

namespace ib {
namespace mw {
namespace registry {

struct MessageHeader
{
    // Integration Bus Participant Announcement
    std::array<char, 5> preambel{{'V', 'I', 'B', 'P', 'A'}};
    uint16_t versionHigh = 1;
    uint16_t versionLow = 0;
};

inline bool operator!=(const registry::MessageHeader& lhs, const registry::MessageHeader& rhs)
{
    return lhs.preambel != rhs.preambel
        || lhs.versionHigh != rhs.versionHigh
        || lhs.versionLow != rhs.versionLow;
}

struct ParticipantAnnouncement
{
    MessageHeader messageHeader;
    ib::mw::VAsioPeerInfo peerInfo;
};

struct KnownParticipants
{
    MessageHeader messageHeader;
    std::vector<ib::mw::VAsioPeerInfo> peerInfos;
};

enum class RegistryMessageKind : uint8_t
{
    Invalid = 0,
    ParticipantAnnouncement = 1,
    KnownParticipants = 2,
    SubscriptionSent = 3
};

} // namespace registry
} // namespace mw
} // namespace ib
