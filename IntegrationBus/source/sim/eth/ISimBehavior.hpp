// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/eth/EthernetDatatypes.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const Core::IServiceEndpoint* from) const -> bool = 0;
    virtual void SendMsg(EthernetFrameEvent&& msg) = 0;
    virtual void SendMsg(EthernetSetMode&& msg) = 0;

    virtual void OnReceiveAck(const EthernetFrameTransmitEvent& msg) = 0;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
