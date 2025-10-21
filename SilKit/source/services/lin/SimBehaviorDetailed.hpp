// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    void SendBroadcastMsgImpl(MsgT&& msg);

    template <typename MsgT>
    void SendTargettedMsgImpl(MsgT&& msg);

    Core::IParticipantInternal* _participant{nullptr};
    LinController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    const Core::ServiceDescriptor* _parentServiceDescriptor{nullptr};
    Core::ServiceDescriptor _simulatedLink;
};


} // namespace Lin
} // namespace Services
} // namespace SilKit
