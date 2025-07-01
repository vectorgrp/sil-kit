// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateSilKitRegistryWithDashboard.hpp"

#include "VAsioRegistry.hpp"


namespace VSilKit {


auto CreateSilKitRegistryWithDashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                                       SilKit::Core::IRegistryEventListener* registryEventListener)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>
{
    return std::make_unique<SilKit::Core::VAsioRegistry>(config, registryEventListener);
}


} // namespace VSilKit
