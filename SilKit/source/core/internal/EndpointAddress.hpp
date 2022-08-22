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
    return lhs.participant == rhs.participant
        && lhs.endpoint == rhs.endpoint;
}

bool operator!=(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs)
{
    return lhs.participant != rhs.participant
        || lhs.endpoint != rhs.endpoint;
}

bool operator<(SilKit::Core::EndpointAddress lhs, SilKit::Core::EndpointAddress rhs)
{
    return lhs.participant < rhs.participant
        || (lhs.participant == rhs.participant && lhs.endpoint < rhs.endpoint);
}

} // namespace Core
} // namespace SilKit
