#pragma once
#include <memory>
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "IDashboard.hpp"

namespace SilKitRegistry {

struct RegistryInstance
{
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::unique_ptr<SilKit::Dashboard::IDashboard> _dashboard;
};

} //end SilKitRegistry
