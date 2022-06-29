// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/LinDatatypes.hpp"

#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace lin {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool = 0;

    virtual void SendIbMessage(LinSendFrameRequest&& sendFrameRequest) = 0;
    virtual void SendIbMessage(LinTransmission&& transmission) = 0;
    virtual void SendIbMessage(LinControllerConfig&& controllerConfig) = 0;
    virtual void SendIbMessage(LinSendFrameHeaderRequest&& header) = 0;
    virtual void SendIbMessage(LinFrameResponseUpdate&& frameResponseUpdate) = 0;
    virtual void SendIbMessage(LinControllerStatusUpdate&& statusUpdate) = 0;
    
    virtual void GoToSleep() = 0;
    virtual void Wakeup() = 0;

    virtual auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus = 0;
};

} // namespace lin
} // namespace sim
} // namespace ib
