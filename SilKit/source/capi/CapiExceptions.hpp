// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/participant/exception.hpp"

namespace SilKit {

struct CapiBadParameterError : SilKitError
{
    using SilKitError::SilKitError;
};

} // namespace SilKit
