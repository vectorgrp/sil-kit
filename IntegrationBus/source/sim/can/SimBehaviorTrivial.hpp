// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbToCanController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace ib {
namespace sim {
namespace can {

class CanController;

class SimBehaviorTrivial : public ISimBehavior
{
public:

    SimBehaviorTrivial(mw::IParticipantInternal* participant, CanController* canController,
                      mw::sync::ITimeProvider* timeProvider);

    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;
    void SendIbMessage(CanConfigureBaudrate&& /*baudRate*/) override;
    void SendIbMessage(CanSetControllerMode&& mode) override;
    void SendIbMessage(CanFrameEvent&& canFrameEvent) override;

private:
    template <typename MsgT>
    void ReceiveIbMessage(const MsgT& msg);

    mw::IParticipantInternal* _participant{nullptr};
    CanController* _parentController{nullptr};
    const mw::IIbServiceEndpoint* _parentServiceEndpoint{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

} // namespace can
} // namespace sim
} // namespace ib
