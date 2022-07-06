// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
                       Core::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    
    void SendMsg(LinSendFrameRequest&& sendFrameRequest) override;
    void SendMsg(LinTransmission&& transmission) override;
    void SendMsg(LinControllerConfig&& controllerConfig) override;
    void SendMsg(LinSendFrameHeaderRequest&& header) override;
    void SendMsg(LinFrameResponseUpdate&& frameResponseUpdate) override;
    void SendMsg(LinControllerStatusUpdate&& statusUpdate) override;

    void GoToSleep() override;
    void Wakeup() override;
    auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus override;

private:
    template <typename MsgT>
    void ReceiveSilKitMessage(const MsgT& msg);

    template <typename MsgT>
    void SendMsgImpl(MsgT&& msg);

    Core::IParticipantInternal* _participant{nullptr};
    LinController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Tracer _tracer;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
