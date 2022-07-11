// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IMsgForEthController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class EthController;

class SimBehaviorTrivial : public ISimBehavior
{
public:

    SimBehaviorTrivial(Core::IParticipantInternal* participant, EthController* ethController,
                      Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(EthernetFrameEvent&& ethFrameEvent) override;
    void SendMsg(EthernetSetMode&& ethFrameEvent) override;

    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;

private:
    template <typename MsgT>
    void ReceiveMsg(const MsgT& msg);

    Core::IParticipantInternal* _participant{nullptr};
    EthController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Tracer _tracer;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
