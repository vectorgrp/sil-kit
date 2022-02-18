// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
//! This is the shared library version of the registry. It is accessible through
//  the ib/extensions/LoadExtension.hpp API.

#include <memory>

#include "ib/version.hpp"

#include "VAsioRegistry.hpp"

#include "IbExtensionBase.hpp"
#include "IbExtensionMacros.hpp"
#include "IIbRegistry.hpp"


//This factory class exposes a parameterized CTor of VAsioRegistry
//and a corresponding deleter.
class IbRegistryFactory
    : public IIbRegistryFactory
    , public ib::extensions::IbExtensionBase
{
public:
    //public IIbRegistryFactory methods
    auto Create(std::shared_ptr<ib::cfg::vasio::IMiddlewareConfiguration> cfg)
        -> std::unique_ptr<ib::extensions::IIbRegistry> override;

};

auto IbRegistryFactory::Create(std::shared_ptr<ib::cfg::vasio::IMiddlewareConfiguration> cfg)
        -> std::unique_ptr<ib::extensions::IIbRegistry>
{
    return std::make_unique<ib::mw::VAsioRegistry>(cfg);
}


IB_DECLARE_EXTENSION(
    IbRegistryFactory,
    "Vector Informatik",
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch()
)

