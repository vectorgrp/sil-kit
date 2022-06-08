// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/eth/EthernetDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace eth {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool = 0;
    virtual void SendIbMessage(EthernetFrameEvent&& msg) = 0;
    virtual void SendIbMessage(EthernetSetMode&& msg) = 0;

    virtual void OnReceiveAck(const EthernetFrameTransmitEvent& msg) = 0;
};

} // namespace eth
} // namespace sim
} // namespace ib
