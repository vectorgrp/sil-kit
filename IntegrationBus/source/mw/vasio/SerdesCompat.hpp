// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include <string>
#include <vector>

#include "MessageBuffer.hpp"
#include "VAsioDatatypes.hpp"

namespace ib {
namespace mw {
//! Backward compatibility for the complete Handshake sequence
void DeserializeCompat(MessageBuffer&, ParticipantAnnouncement&);
void SerializeCompat(MessageBuffer&, const ParticipantAnnouncement&);
void DeserializeCompat(MessageBuffer&, KnownParticipants&);
void SerializeCompat(MessageBuffer&, const KnownParticipants&);
void DeserializeCompat(MessageBuffer&, ParticipantAnnouncementReply&);
void SerializeCompat(MessageBuffer&, const ParticipantAnnouncementReply&);
} //mw
} //ib
