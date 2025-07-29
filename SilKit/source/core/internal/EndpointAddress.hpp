// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cstdint>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Core {

/*! \brief Deprecated identifier for SIL Kit participants
 * Will be fully replaced by participant name in future versions.
*/
using ParticipantId = uint64_t;

/*! \brief Participant specific identifier for its communication endpoints.
 *
 *   One SIL Kit participant can have multiple communication objects,
 *   e.g., multiple CAN controllers attached to different buses. A EndpointId
 *   is only valid in the scope of a specific Instruction Bus participant.
 */
using EndpointId = uint64_t;

/*! \brief Global address of a ComEndpoint
 *
 *  Pair of ParticipantId and EndpointId.
 */
struct EndpointAddress
{
    ParticipantId participant;
    EndpointId endpoint;
};

inline bool operator==(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs);
inline bool operator!=(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs);
inline bool operator<(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs);

// ================================================================================
//  Inline Implementations
// ================================================================================
bool operator==(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs)
{
    return lhs.participant == rhs.participant && lhs.endpoint == rhs.endpoint;
}

bool operator!=(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs)
{
    return lhs.participant != rhs.participant || lhs.endpoint != rhs.endpoint;
}

bool operator<(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs)
{
    return lhs.participant < rhs.participant || (lhs.participant == rhs.participant && lhs.endpoint < rhs.endpoint);
}

} // namespace Core
} // namespace SilKit
