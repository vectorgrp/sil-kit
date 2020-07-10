// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "CreateInstance.hpp"


namespace ib { namespace extensions {

auto CreateIbRegistry(ib::cfg::Config config)
    -> ExtensionHandle<IIbRegistry>
{
    return CreateInstance<IIbRegistryFactory, IIbRegistry>
        ("vib-registry", std::move(config));
}

}//end namespace extensions
}//end namespace ib
