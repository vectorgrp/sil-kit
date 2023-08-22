/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#pragma once
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"

namespace SilKit {
namespace Core {

// Associate a Message Kind with concrete VAsio data types.
// The VAsioMsgKind tags are encoded in serialized (network) messages.

// default is SilKitMwMsg
template<typename MessageT>
inline constexpr auto messageKind() -> VAsioMsgKind { return VAsioMsgKind::SilKitMwMsg; }

// Registry messages
template<>
inline constexpr auto messageKind<ParticipantAnnouncement>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<ParticipantAnnouncementReply>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<KnownParticipants>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }
template<>
inline constexpr auto messageKind<RemoteParticipantConnectRequest>() -> VAsioMsgKind { return VAsioMsgKind::SilKitRegistryMessage; }

// Service subscription
template<>
inline constexpr auto messageKind<SubscriptionAcknowledge>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAcknowledge; }
template<>
inline constexpr auto messageKind<VAsioMsgSubscriber>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAnnouncement; }

// Proxy messages
template<>
inline constexpr auto messageKind<ProxyMessage>() -> VAsioMsgKind { return VAsioMsgKind::SilKitProxyMessage; }

template<typename MessageT>
inline constexpr auto registryMessageKind() -> RegistryMessageKind { return RegistryMessageKind::Invalid; }
template<>
inline constexpr auto registryMessageKind<ParticipantAnnouncement>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncement; }
template<>
inline constexpr auto registryMessageKind<ParticipantAnnouncementReply>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncementReply; }
template<>
inline constexpr auto registryMessageKind<KnownParticipants>() -> RegistryMessageKind { return RegistryMessageKind::KnownParticipants; }
template<>
inline constexpr auto registryMessageKind<RemoteParticipantConnectRequest>() -> RegistryMessageKind
{
    return RegistryMessageKind::RemoteParticipantConnectRequest;
}

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
