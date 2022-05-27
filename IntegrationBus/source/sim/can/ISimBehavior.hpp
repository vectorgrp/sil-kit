// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/can/CanDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace can {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool = 0;
    virtual void SendIbMessage(CanConfigureBaudrate&& msg) = 0;
    virtual void SendIbMessage(CanSetControllerMode&& msg) = 0;
    virtual void SendIbMessage(CanFrameEvent&& msg) = 0;
};

} // namespace can
} // namespace sim
} // namespace ib
