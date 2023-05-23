/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
