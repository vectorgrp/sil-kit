// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Replaces the whole dashboard implementation when SILKIT_BUILD_DASHBOARD is OFF. Callers should
// consult IsDashboardAvailable() first; the throwing factory is only a backstop.

#include "dashboard/CreateDashboardInstance.hpp"
#include "silkit/participant/exception.hpp"

namespace VSilKit {

auto IsDashboardAvailable() -> bool
{
    return false;
}

auto CreateDashboardInstance(const std::string& /*dashboardUri*/) -> std::unique_ptr<IDashboardInstance>
{
    throw SilKit::SilKitError("SIL Kit Dashboard support is disabled");
}

} // namespace VSilKit
