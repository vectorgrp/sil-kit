// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include "silkit/capi/Parameters.h"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

class ParameterProvider
{
public:
    inline ParameterProvider();

    inline ~ParameterProvider() = default;

    inline auto GetParameter(SilKit_Participant* participant, Parameter parameter) -> std::string;

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

auto ParameterProvider::GetParameter(SilKit_Participant* participant, Parameter parameter) -> std::string
{
    std::vector<char> buffer;
    size_t size = 0;
    SilKit_Parameter cParameter = static_cast<SilKit_Parameter>(parameter);
    {
        const auto returnCode = SilKit_Participant_GetParameter(nullptr, &size, cParameter, participant);
        ThrowOnError(returnCode);
    }
    while (size > buffer.size())
    {
        buffer.resize(size);
        const auto returnCode = SilKit_Participant_GetParameter(buffer.data(), &size, cParameter, participant);
        ThrowOnError(returnCode);
    }
    buffer.resize(size);

    return std::string{buffer.data()};
}

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
