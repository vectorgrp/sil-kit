// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "FactorySingleton.hpp"
#include "vasio/MiddlewareConfiguration.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib { namespace extensions {

auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>
{
    // Preliminary convert old config
    ib::cfg::v1::datatypes::Extensions dummyExtentionCfg{config.extensionConfig.searchPathHints};
    auto dummyCfg = ib::cfg::vasio::v1::CreateDummyIMiddlewareConfiguration();

    auto& factory = FactorySingleton<IIbRegistryFactory>("vib-registry", dummyExtentionCfg);
    return factory.Create(std::move(dummyCfg));
}

    

}//end namespace extensions
}//end namespace ib
