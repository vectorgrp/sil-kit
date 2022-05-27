// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbToCanController.hpp"
#include "IParticipantInternal.hpp"

#include "ISimBehavior.hpp"

namespace ib {
namespace sim {
namespace can {

class CanController;

class SimBehaviorDetailed : public ISimBehavior
{
public:
    SimBehaviorDetailed(mw::IParticipantInternal* participant, CanController* canController,
                       const mw::ServiceDescriptor& serviceDescriptor);

    void SendIbMessage(CanConfigureBaudrate&& msg) override;
    void SendIbMessage(CanSetControllerMode&& msg) override;
    void SendIbMessage(CanFrameEvent&& msg) override;
    
    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;

    void SetSimulatedLink(const mw::ServiceDescriptor& simulatedLink);

private:
    template <typename MsgT>
    inline void SendIbMessageImpl(MsgT&& msg);

    mw::IParticipantInternal* _participant{nullptr};
    ICanController* _parentController{nullptr};
    const mw::IIbServiceEndpoint* _parentServiceEndpoint{nullptr};
    const mw::ServiceDescriptor* _parentServiceDescriptor{nullptr};
    mw::ServiceDescriptor _simulatedLink;
};


} // namespace can
} // namespace sim
} // namespace ib
