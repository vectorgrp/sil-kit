// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForLinController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController;

class SimBehaviorTrivial : public ISimBehavior
{
public:
    SimBehaviorTrivial(Core::IParticipantInternal* participant, LinController* linController,
                       Services::Orchestration::ITimeProvider* timeProvider);

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

private:
    template <typename MsgT>
    void ReceiveMsg(const MsgT& msg);

    template <typename MsgT>
    void SendMsgImpl(MsgT&& msg);

    void SendErrorTransmissionOnHeaderRequest(int numResponses, LinFrame frame);

    Core::IParticipantInternal* _participant{nullptr};
    LinController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
