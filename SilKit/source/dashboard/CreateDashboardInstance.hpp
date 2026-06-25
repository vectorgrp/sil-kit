// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "dashboard/IDashboardInstance.hpp"

#include <memory>


namespace VSilKit {


auto CreateDashboardInstance() -> std::unique_ptr<IDashboardInstance>;


} // namespace VSilKit
