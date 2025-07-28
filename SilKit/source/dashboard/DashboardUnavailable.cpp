// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateDashboardInstance.hpp"
#include "silkit/participant/exception.hpp"

namespace VSilKit {

auto CreateDashboardInstance() -> std::unique_ptr<IDashboardInstance>
{
    throw SilKit::SilKitError("SIL Kit Dashboard support is disabled");
}

} // namespace VSilKit
