// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    void SendMsg(WireEthernetFrameEvent&& ethFrameEvent) override;
    void SendMsg(EthernetSetMode&& ethFrameEvent) override;

    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;

private:
    template <typename MsgT>
    void ReceiveMsg(const MsgT& msg);

    Core::IParticipantInternal* _participant{nullptr};
    EthController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
