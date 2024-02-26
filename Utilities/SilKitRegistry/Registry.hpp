// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/vendor/ISilKitRegistry.hpp"
#include "IDashboardInstance.hpp"

#include <memory>


namespace SilKitRegistry {


struct RegistryInstance
{
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::unique_ptr<VSilKit::IDashboardInstance> _dashboard;
};


} //end SilKitRegistry
