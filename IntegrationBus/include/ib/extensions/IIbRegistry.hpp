// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <cstdint>
#include <functional>

namespace ib { namespace extensions {

//! \brief Dedicated IB registry for the VAsio middleware.
//         This is a loadable runtime extension that is non-redistributable
class IIbRegistry
{
public:
    virtual ~IIbRegistry() = default;
    virtual void ProvideDomain(uint32_t domainId) = 0;
    virtual void SetAllConnectedHandler(std::function<void()> handler) = 0;
    virtual void SetAllDisconnectedHandler(std::function<void()> handler) = 0;
};


}//end namespace extensions
}//end namespace ib
