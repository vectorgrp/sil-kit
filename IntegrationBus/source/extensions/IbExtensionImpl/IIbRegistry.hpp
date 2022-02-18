// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
//
#pragma once

#include <memory>

#include "ib/extensions/IIbRegistry.hpp"
#include "ib/cfg/vasio/IMiddlewareConfiguration.hpp"

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
    virtual auto Create(std::shared_ptr<ib::cfg::vasio::IMiddlewareConfiguration> cfg)
        -> std::unique_ptr<ib::extensions::IIbRegistry>  = 0;
};
