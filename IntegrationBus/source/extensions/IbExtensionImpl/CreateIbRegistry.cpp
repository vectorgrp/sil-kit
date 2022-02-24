// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "FactorySingleton.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace extensions {

auto CreateIbRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>
{
    auto cfg = std::dynamic_pointer_cast<cfg::datatypes::ParticipantConfiguration>(config);
    auto& factory = FactorySingleton<IIbRegistryFactory>("vib-registry", cfg->extensions);
    return factory.Create(std::move(cfg));
}

} // namespace extensions
} // namespace ib
