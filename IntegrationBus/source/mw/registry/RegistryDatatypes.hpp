// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <array>

#include "../vasio/VAsioPeerInfo.hpp"

namespace ib {
namespace mw {
namespace registry {

struct RegistryMessageHeader
{
    // Integration Bus Participant Announcment
    std::array<char, 4> preambel{{'I', 'B', 'P', 'A'}};
    uint16_t versionHigh = 1;
    uint16_t versionLow = 0;
};

inline bool operator!=(const registry::RegistryMessageHeader& lhs, const registry::RegistryMessageHeader& rhs)
{
    return lhs.preambel != rhs.preambel
        || lhs.versionHigh != rhs.versionHigh
        || lhs.versionLow != rhs.versionLow;
}

struct ParticipantAnnouncement
    : public RegistryMessageHeader
{
    ib::mw::VAsioPeerInfo localInfo;
};

struct KnownParticipants
    : public RegistryMessageHeader
{
    std::vector<ib::mw::VAsioPeerInfo> participantInfos;
};

enum class RegistryMessageKind
{
    Invalid = 0,
    ParticipantAnnouncement = 1,
    KnownParticipants = 2,
    SubscriptionSent = 3
};

} // namespace registry
} // namespace mw
} // namespace ib
