// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>
#include <memory>

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/core/logging/ILogger.hpp"

#include "ParticipantConfiguration.hpp"
#include "VAsioRegistry.hpp"


namespace SilKit {
namespace Vendor {
inline namespace Vector {

auto CreateSilKitRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
    -> std::unique_ptr<Vendor::ISilKitRegistry>
{
    return std::make_unique<Core::VAsioRegistry>(config);
}


} // namespace Vector
} // namespace Vendor
} // namespace SilKit
