// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <ostream>

#include "OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs);
bool operator==(const SystemCommand& lhs, const SystemCommand& rhs);
bool operator==(const WorkflowConfiguration& lhs, const WorkflowConfiguration& rhs);
bool operator==(const ParticipantConnectionInformation& lhs, const ParticipantConnectionInformation& rhs);

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
