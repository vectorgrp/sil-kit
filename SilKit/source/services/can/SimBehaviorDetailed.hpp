// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForCanController.hpp"
#include "IParticipantInternal.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController;

class SimBehaviorDetailed : public ISimBehavior
{
public:
    SimBehaviorDetailed(Core::IParticipantInternal* participant, CanController* canController,
                        const Core::ServiceDescriptor& serviceDescriptor);

    void SendMsg(CanConfigureBaudrate&& msg) override;
    void SendMsg(CanSetControllerMode&& msg) override;
    void SendMsg(WireCanFrameEvent&& msg) override;

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;

    void SetSimulatedLink(const Core::ServiceDescriptor& simulatedLink);

private:
    template <typename MsgT>
    void SendMsgImpl(MsgT&& msg);

    Core::IParticipantInternal* _participant{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    const Core::ServiceDescriptor* _parentServiceDescriptor{nullptr};
    Core::ServiceDescriptor _simulatedLink;
    Tracer* _tracer{nullptr};
};


} // namespace Can
} // namespace Services
} // namespace SilKit
