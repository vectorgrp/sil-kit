// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include "silkit/capi/Parameters.h"
#include "silkit/participant/exception.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

class ParameterProvider
{
public:
    inline ParameterProvider();

    inline ~ParameterProvider() = default;

    inline auto GetStringParameter(SilKit_Participant* participant, Parameter parameter) -> std::string;

private:
    using StringQueryFunction = SilKit_ReturnCode(SilKitCALL*)(void*, size_t*, SilKit_Participant*);
    inline auto QueryString(SilKit_Participant* participant, StringQueryFunction stringQueryFunction) -> std::string;

};

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

ParameterProvider::ParameterProvider()
{
}

auto ParameterProvider::QueryString(SilKit_Participant* participant,
                                    StringQueryFunction stringQueryFunction) -> std::string
{
    std::vector<char> buffer;
    size_t size = 0;

    {
        const auto returnCode = stringQueryFunction(nullptr, &size, participant);
        ThrowOnError(returnCode);
    }

    while (size > buffer.size())
    {
        buffer.resize(size);
        const auto returnCode = stringQueryFunction(buffer.data(), &size, participant);
        ThrowOnError(returnCode);
    }

    return std::string{buffer.data()};
}

auto ParameterProvider::GetStringParameter(SilKit_Participant* participant, Parameter parameter) -> std::string
{
    switch (parameter)
    {
    case Parameter::ParticipantName:
        return QueryString(participant, &SilKit_Participant_GetParticipantName);
    case Parameter::RegistryUri:
        return QueryString(participant, &SilKit_Participant_GetGetRegistryUri);
    default:
        throw SilKit::SilKitError("Unknown parameter.");
    }
}

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
