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

#include "IMsgForLinController.hpp"
#include "IParticipantInternal.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController;

class SimBehaviorDetailed : public ISimBehavior
{
public:
    SimBehaviorDetailed(Core::IParticipantInternal* participant, LinController* linController,
                       const Core::ServiceDescriptor& serviceDescriptor);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;

    void SendMsg(LinSendFrameRequest&& sendFrameRequest) override;
    void SendMsg(LinTransmission&& transmission) override;
    void SendMsg(WireLinControllerConfig&& controllerConfig) override;
    void SendMsg(LinSendFrameHeaderRequest&& header) override;
    void SendMsg(LinFrameResponseUpdate&& frameResponseUpdate) override;
    void SendMsg(LinControllerStatusUpdate&& statusUpdate) override;

    void ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& header) override;

    void UpdateTxBuffer(const LinFrame& frame) override;

    void GoToSleep() override;
    void Wakeup() override;
    auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus override;

    void SetSimulatedLink(const Core::ServiceDescriptor& simulatedLink);

private:
    template <typename MsgT>
    void SendMsgImpl(MsgT&& msg);

    Core::IParticipantInternal* _participant{nullptr};
    LinController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    const Core::ServiceDescriptor* _parentServiceDescriptor{nullptr};
    Core::ServiceDescriptor _simulatedLink;
};


} // namespace Lin
} // namespace Services
} // namespace SilKit
