#pragma once
#include "ib/extensions/IIbRegistry.hpp"
#include "ib/cfg/Config.hpp"

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
    virtual ib::extensions::IIbRegistry* Create(ib::cfg::Config) = 0;
    virtual void Release(ib::extensions::IIbRegistry*) = 0;
};
