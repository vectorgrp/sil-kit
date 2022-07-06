// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <ostream>

#include "SyncDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

bool operator==(const ParticipantCommand& lhs, const ParticipantCommand& rhs);
bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs);
bool operator==(const SystemCommand& lhs, const SystemCommand& rhs);
bool operator==(const WorkflowConfiguration& lhs, const WorkflowConfiguration& rhs);

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
