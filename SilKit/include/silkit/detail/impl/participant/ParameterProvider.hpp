// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "silkit/participant/exception.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

using StringQueryFunction = SilKit_ReturnCode(SilKitCALL*)(char*, size_t*, SilKit_Participant*);
inline auto QueryString(SilKit_Participant* participant, StringQueryFunction stringQueryFunction) -> std::string;

inline auto GetParticipantName(SilKit_Participant* participant) -> std::string;
inline auto GetRegistryUri(SilKit_Participant* participant) -> std::string;

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

auto QueryString(SilKit_Participant* participant, StringQueryFunction stringQueryFunction) -> std::string
{
    std::vector<char> buffer;
    size_t size = 0;

    {
        // Initially only get the size of the string to be queried
        const auto returnCode = stringQueryFunction(nullptr, &size, participant);
        ThrowOnError(returnCode);
    }

    while (size > buffer.size())
    {
        // Retry until the returned required size fits our buffer:
        // the value may change between the initial size query and the copy call.
        buffer.resize(size);
        const auto returnCode = stringQueryFunction(buffer.data(), &size, participant);
        ThrowOnError(returnCode);
    }

    return std::string{buffer.data()};
}

auto GetParticipantName(SilKit_Participant* participant) -> std::string
{
    return QueryString(participant, &SilKit_Participant_GetParticipantName);
}

auto GetRegistryUri(SilKit_Participant* participant) -> std::string
{
    return QueryString(participant, &SilKit_Participant_GetRegistryUri);
}

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
