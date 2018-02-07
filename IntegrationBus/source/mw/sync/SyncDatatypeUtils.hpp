// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <ostream>

#include "ib/mw/sync/SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

bool operator==(const QuantumRequest& lhs, const QuantumRequest& rhs);
bool operator==(const QuantumGrant& lhs, const QuantumGrant& rhs);
bool operator==(const Tick& lhs, const Tick& rhs);
bool operator==(const ParticipantCommand& lhs, const ParticipantCommand& rhs);
bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs);
bool operator==(const SystemCommand& lhs, const SystemCommand& rhs);

}
}
}
