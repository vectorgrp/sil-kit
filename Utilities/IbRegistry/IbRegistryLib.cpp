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
    : public IIbRegistryFactory
    , public ib::extensions::IbExtensionBase
{
public:
    //public IIbRegistryFactory methods
    auto Create(ib::cfg::Config )
        -> ib::extensions::IIbRegistry* override;
    void Release(ib::extensions::IIbRegistry*) override;

};

auto IbRegistryFactory::Create(ib::cfg::Config cfg)
        -> ib::extensions::IIbRegistry*
{
    return new ib::mw::VAsioRegistry(std::move(cfg));
}

void IbRegistryFactory::Release(ib::extensions::IIbRegistry* inst)
{
    delete inst;
}

IB_DECLARE_EXTENSION(
    IbRegistryFactory,
    "Vector Informatik",
    ib::version::Major(),
    ib::version::Minor(),
    ib::version::Patch()
)

