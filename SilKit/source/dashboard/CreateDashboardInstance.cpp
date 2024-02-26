// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateDashboardInstance.hpp"
#include "DashboardInstance.hpp"

namespace VSilKit {


auto CreateDashboardInstance() -> std::unique_ptr<IDashboardInstance>
{
    return std::make_unique<DashboardInstance>();
}


} // namespace VSilKit
