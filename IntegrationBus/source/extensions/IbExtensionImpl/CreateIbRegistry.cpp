// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "FactorySingleton.hpp"


namespace ib { namespace extensions {

auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>
{
    auto& factory = FactorySingleton<IIbRegistryFactory>("vib-registry", config);
    return factory.Create(std::move(config));
}

}//end namespace extensions
}//end namespace ib
