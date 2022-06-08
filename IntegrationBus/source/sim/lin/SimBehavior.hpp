// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "SimBehaviorDetailed.hpp"
#include "SimBehaviorTrivial.hpp"

#include "ib/sim/lin/LinDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(mw::IParticipantInternal* participant, LinController* linController,
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

    void SetDetailedBehavior(const mw::ServiceDescriptor& simulatedLink);
    void SetTrivialBehavior();

    auto IsTrivial() const -> bool;
    auto IsDetailed() const -> bool;

private:
    template <typename MsgT>
    void SendIbMessageImpl(MsgT&& msg);

    SimBehaviorTrivial _trivial;
    SimBehaviorDetailed _detailed;
    ISimBehavior* _currentBehavior;
};

} // namespace lin
} // namespace sim
} // namespace ib
