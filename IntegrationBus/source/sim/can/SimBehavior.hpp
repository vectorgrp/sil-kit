// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "SimBehaviorDetailed.hpp"
#include "SimBehaviorTrivial.hpp"

#include "ib/sim/can/CanDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace can {

class CanController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(mw::IParticipantInternal* participant, CanController* canController,
                       mw::sync::ITimeProvider* timeProvider);

    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;
    void SendIbMessage(CanConfigureBaudrate&& msg) override;
    void SendIbMessage(CanSetControllerMode&& msg) override;
    void SendIbMessage(CanFrameEvent&& msg) override;

    void SetDetailedBehavior(const mw::ServiceDescriptor& simulatedLink);
    void SetTrivialBehavior();

    auto IsTrivial() const -> bool;
    auto IsDetailed() const -> bool;

private:
    template <typename MsgT>
    inline void SendIbMessageImpl(MsgT&& msg);

    SimBehaviorTrivial _trivial;
    SimBehaviorDetailed _detailed;
    ISimBehavior* _currentStrategy;
};

} // namespace can
} // namespace sim
} // namespace ib
