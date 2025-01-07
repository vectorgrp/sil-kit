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

    inline auto GetParameter(SilKit_Participant* participant, Parameter parameter) -> const std::string&;

private:
    std::unordered_map<Parameter, std::string> _parameterValues;
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

auto ParameterProvider::GetParameter(SilKit_Participant* participant, Parameter parameter) -> const std::string&
{
    const char* cParameterValue;
    SilKit_Parameter cParameter = static_cast<SilKit_Parameter>(parameter);
    const auto returnCode = SilKit_Participant_GetParameter(&cParameterValue, cParameter, participant);
    _parameterValues[parameter] = std::string(cParameterValue);
    ThrowOnError(returnCode);
    return _parameterValues[parameter];
}

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
