// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include <string>
#include <vector>

#include "MessageBuffer.hpp"
#include "VAsioDatatypes.hpp"

namespace ib {
namespace mw {
void DeserializeCompat(MessageBuffer&, ParticipantAnnouncement&);
} //mw
} //ib
