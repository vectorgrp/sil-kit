// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <string>


namespace SilKit {
namespace Core {
struct IRegistryEventListener;
} // namespace Core
} // namespace SilKit


namespace VSilKit {


struct IDashboardInstance
{
    virtual ~IDashboardInstance() = default;

    virtual auto GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener* = 0;
    virtual void SetupDashboardConnection(const std::string& dashboardUri) = 0;
};


} // namespace VSilKit