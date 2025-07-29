// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "VAsioSerdes.hpp"

namespace SilKit {
namespace Core {

void SerializeV30(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply);
void DeserializeV30(MessageBuffer& buffer, ParticipantAnnouncementReply& reply);

void SerializeV30(MessageBuffer& buffer, const ParticipantAnnouncement& reply);
void DeserializeV30(MessageBuffer& buffer, ParticipantAnnouncement& reply);

void SerializeV30(MessageBuffer& buffer, const KnownParticipants& reply);
void DeserializeV30(MessageBuffer& buffer, KnownParticipants& reply);

} // namespace Core
} // namespace SilKit
