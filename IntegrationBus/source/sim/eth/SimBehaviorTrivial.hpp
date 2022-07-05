// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbToEthController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthController;

class SimBehaviorTrivial : public ISimBehavior
{
public:

    SimBehaviorTrivial(mw::IParticipantInternal* participant, EthController* ethController,
                      mw::sync::ITimeProvider* timeProvider);

    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;
    void SendIbMessage(EthernetFrameEvent&& ethFrameEvent) override;
    void SendIbMessage(EthernetSetMode&& ethFrameEvent) override;

    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;

private:
    template <typename MsgT>
    void ReceiveIbMessage(const MsgT& msg);

    mw::IParticipantInternal* _participant{nullptr};
    EthController* _parentController{nullptr};
    const mw::IIbServiceEndpoint* _parentServiceEndpoint{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

} // namespace eth
} // namespace sim
} // namespace ib
