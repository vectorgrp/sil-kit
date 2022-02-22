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

auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>
{
    // Preliminary convert old config
    ib::cfg::v1::datatypes::Extensions dummyExtensionCfg{config.extensionConfig.searchPathHints};
    auto dummyCfg = std::make_shared<ib::cfg::v1::datatypes::ParticipantConfiguration>();

    auto& factory = FactorySingleton<IIbRegistryFactory>("vib-registry", dummyExtensionCfg);
    return factory.Create(std::move(dummyCfg));
}

} // namespace extensions
} // namespace ib
