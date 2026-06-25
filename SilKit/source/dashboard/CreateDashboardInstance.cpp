// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/CreateDashboardInstance.hpp"
#include "dashboard/DashboardInstance.hpp"

namespace VSilKit {


auto CreateDashboardInstance() -> std::unique_ptr<IDashboardInstance>
{
    return std::make_unique<DashboardInstance>();
}


} // namespace VSilKit
