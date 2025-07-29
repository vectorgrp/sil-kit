// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/lin/LinDatatypes.hpp"
#include "WireLinMessages.hpp"

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const Core::IServiceEndpoint* from) const -> bool = 0;

    virtual void SendMsg(LinSendFrameRequest&& sendFrameRequest) = 0;
    virtual void SendMsg(LinTransmission&& transmission) = 0;
    virtual void SendMsg(WireLinControllerConfig&& controllerConfig) = 0;
    virtual void SendMsg(LinSendFrameHeaderRequest&& header) = 0;
    virtual void SendMsg(LinFrameResponseUpdate&& frameResponseUpdate) = 0;
    virtual void SendMsg(LinControllerStatusUpdate&& statusUpdate) = 0;

    virtual void ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& header) = 0;

    virtual void UpdateTxBuffer(const LinFrame& frame) = 0;

    virtual void GoToSleep() = 0;
    virtual void Wakeup() = 0;

    virtual auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
