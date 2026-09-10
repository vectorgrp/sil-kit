// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/vendor/ISilKitRegistry.hpp"
#include "dashboard/IDashboardInstance.hpp"

#include <memory>


namespace SilKitRegistry {


/*! Owns a running registry and, optionally, the dashboard attached to it.
 *
 *  Declaration order is load-bearing. VAsioRegistry holds the dashboard as a raw, never-reset
 *  IRegistryEventListener* and keeps delivering events to it from its I/O thread for as long as it
 *  lives, so the registry has to be torn down first. Members are destroyed in reverse declaration
 *  order, hence _dashboard first here and _registry second.
 */
struct RegistryInstance
{
    std::unique_ptr<VSilKit::IDashboardInstance> _dashboard;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
};


} // namespace SilKitRegistry
