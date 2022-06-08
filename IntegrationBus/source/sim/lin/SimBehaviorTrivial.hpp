// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbToLinController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController;

class SimBehaviorTrivial : public ISimBehavior
{
public:

    SimBehaviorTrivial(mw::IParticipantInternal* participant, LinController* linController,
                       mw::sync::ITimeProvider* timeProvider);

    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;
    
    void SendIbMessage(LinSendFrameRequest&& sendFrameRequest) override;
    void SendIbMessage(LinTransmission&& transmission) override;
    void SendIbMessage(LinControllerConfig&& controllerConfig) override;
    void SendIbMessage(LinSendFrameHeaderRequest&& header) override;
    void SendIbMessage(LinFrameResponseUpdate&& frameResponseUpdate) override;
    void SendIbMessage(LinControllerStatusUpdate&& statusUpdate) override;

    void GoToSleep() override;
    void Wakeup() override;
    auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus override;

private:
    template <typename MsgT>
    void ReceiveIbMessage(const MsgT& msg);

    template <typename MsgT>
    void SendIbMessageImpl(MsgT&& msg);

    mw::IParticipantInternal* _participant{nullptr};
    LinController* _parentController{nullptr};
    const mw::IIbServiceEndpoint* _parentServiceEndpoint{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

} // namespace lin
} // namespace sim
} // namespace ib
