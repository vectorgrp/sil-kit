// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <ostream>

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

bool operator==(const ParticipantCommand& lhs, const ParticipantCommand& rhs);
bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs);
bool operator==(const SystemCommand& lhs, const SystemCommand& rhs);
bool operator==(const ExpectedParticipants& lhs, const ExpectedParticipants& rhs);

}
}
}
