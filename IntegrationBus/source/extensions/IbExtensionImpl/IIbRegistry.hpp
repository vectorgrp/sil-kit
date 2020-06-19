#pragma once
#include "ib/extensions/IIbRegistry.hpp"
#include "ib/cfg/Config.hpp"
#include <memory>
//! \brief Creates an instance of IIbRegistry. Used for loading the extension 
//         when there is no direct access to the CTor of the actual extension
//         interface.
// This auxiliary interface is shared between the extension loading mechanism and
// the extension implementation, but hidden from users of the public
// IIbRegistry interface.

class IIbRegistryFactory
{
public:
    virtual ~IIbRegistryFactory() = default;
    virtual auto Create(ib::cfg::Config)
        -> std::unique_ptr<ib::extensions::IIbRegistry>  = 0;
};
