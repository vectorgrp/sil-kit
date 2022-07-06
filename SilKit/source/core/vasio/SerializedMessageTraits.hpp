// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"

namespace SilKit {
namespace Core {

// Associate a Message Kind with concrete VAsio data types.
// The VAsioMsgKind tags are encoded in serialized (network) messages.

template<typename MessageT>
inline constexpr auto messageKind() -> VAsioMsgKind { return VAsioMsgKind::SilKitMwMsg; }
template<>
inline constexpr auto messageKind<ParticipantAnnouncement>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<ParticipantAnnouncementReply>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<KnownParticipants>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<SubscriptionAcknowledge>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAcknowledge; }
template<>
inline constexpr auto messageKind<VAsioMsgSubscriber>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAnnouncement; }

template<typename MessageT>
inline constexpr auto registryMessageKind() -> RegistryMessageKind { return RegistryMessageKind::Invalid; }
template<>
inline constexpr auto registryMessageKind<ParticipantAnnouncement>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncement; }
template<>
inline constexpr auto registryMessageKind<ParticipantAnnouncementReply>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncementReply; }
template<>
inline constexpr auto registryMessageKind<KnownParticipants>() -> RegistryMessageKind { return RegistryMessageKind::KnownParticipants; }

// Helper function to classify simulation messages based on message kind
inline constexpr bool IsMwOrSim(VAsioMsgKind kind);

//////////////////////////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////////////////////////
inline constexpr bool IsMwOrSim(VAsioMsgKind kind)
{
    return kind == VAsioMsgKind::SilKitMwMsg
        || kind == VAsioMsgKind::SilKitSimMsg
        ;
}

} // namespace Core
} // namespace SilKit
