// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForCanController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController;

class SimBehaviorTrivial : public ISimBehavior
{
public:
    SimBehaviorTrivial(Core::IParticipantInternal* participant, CanController* canController,
                       Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(CanConfigureBaudrate&& /*baudRate*/) override;
    void SendMsg(CanSetControllerMode&& mode) override;
    void SendMsg(WireCanFrameEvent&& canFrameEvent) override;

private:
    template <typename MsgT>
    void ReceiveMsg(const MsgT& msg);

    Core::IParticipantInternal* _participant{nullptr};
    CanController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
};

} // namespace Can
} // namespace Services
} // namespace SilKit
