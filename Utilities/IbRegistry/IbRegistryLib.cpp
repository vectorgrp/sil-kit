// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
//! This is the shared library version of the registry. It is accessible through
//  the ib/extensions/LoadExtension.hpp API.

#include "IbExtensionApi/IbExtensionBase.hpp"
#include "IbExtensionApi/IbExtensionMacros.hpp"
#include "IbExtensionImpl/IIbRegistry.hpp"

#include "VAsioRegistry.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/version.hpp"

#include <memory>


//This factory class exposes a parameterized CTor of VAsioRegistry
//and a corresponding deleter.
class IbRegistryFactory
    : public IIbRegistryFactory2
    , public ib::extensions::IbExtensionBase
{
public:
    //public IIbRegistryFactory methods
    auto Create(ib::cfg::Config )
        -> std::unique_ptr<ib::extensions::IIbRegistry> override;

};

auto IbRegistryFactory::Create(ib::cfg::Config cfg)
        -> std::unique_ptr<ib::extensions::IIbRegistry>
{
    return  std::make_unique<ib::mw::VAsioRegistry>(std::move(cfg));
}


IB_DECLARE_EXTENSION(
    IbRegistryFactory,
    "Vector Informatik",
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch()
)

