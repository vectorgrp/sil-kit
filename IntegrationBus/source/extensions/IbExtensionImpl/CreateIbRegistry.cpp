// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "CreateInstance.hpp"


namespace ib { namespace extensions {

//the unique_ptr's deleter is not type-erased, so we have to use the
//IbRegistryProxy to hide the deleters and shared instances of the DLL.

auto CreateIbRegistry(ib::cfg::Config config)
    -> ExtensionHandle<IIbRegistry>
{
    return CreateInstance<IIbRegistryFactory, IIbRegistry>
        ("vib-registry", std::move(config));
}

}//end namespace extensions
}//end namespace ib
