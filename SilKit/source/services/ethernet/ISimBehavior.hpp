// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "IServiceEndpoint.hpp"
#include "WireEthernetMessages.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const Core::IServiceEndpoint* from) const -> bool = 0;
    virtual void SendMsg(WireEthernetFrameEvent&& msg) = 0;
    virtual void SendMsg(EthernetSetMode&& msg) = 0;

    virtual void OnReceiveAck(const EthernetFrameTransmitEvent& msg) = 0;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
