// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SyncDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

bool operator==(const ParticipantStatus& lhs, const ParticipantStatus& rhs)
{
    return lhs.participantName == rhs.participantName && lhs.state == rhs.state && lhs.enterReason == rhs.enterReason
           && lhs.enterTime == rhs.enterTime && lhs.refreshTime == rhs.refreshTime;
}

bool operator==(const SystemCommand& lhs, const SystemCommand& rhs)
{
    return lhs.kind == rhs.kind;
}

bool operator==(const WorkflowConfiguration& lhs, const WorkflowConfiguration& rhs)
{
    return lhs.requiredParticipantNames == rhs.requiredParticipantNames;
}

bool operator==(const ParticipantConnectionInformation& lhs, const ParticipantConnectionInformation& rhs)
{
    return lhs.participantName == rhs.participantName;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
