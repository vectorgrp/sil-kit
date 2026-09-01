// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include "dashboard/IDashboardInstance.hpp"

namespace VSilKit {

/*! Whether this build has dashboard support compiled in (the SILKIT_BUILD_DASHBOARD option).
 *
 *  Lets callers report a plainly disabled feature without going through an exception.
 */
auto IsDashboardAvailable() -> bool;

/*! Create a dashboard instance that will report to dashboardUri.
 *
 *  Throws if this build has no dashboard support, or if dashboardUri is not a usable http URI.
 *  The instance does not connect until the registry reports its URI.
 */
auto CreateDashboardInstance(const std::string& dashboardUri) -> std::unique_ptr<IDashboardInstance>;

} // namespace VSilKit
