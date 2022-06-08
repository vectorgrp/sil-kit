// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "SimBehaviorDetailed.hpp"
#include "SimBehaviorTrivial.hpp"

#include "ib/sim/eth/EthernetDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(mw::IParticipantInternal* participant, EthController* ethController,
                       mw::sync::ITimeProvider* timeProvider);

    auto AllowReception(const mw::IIbServiceEndpoint* from) const -> bool override;
    void SendIbMessage(EthernetFrameEvent&& msg) override;
    void SendIbMessage(EthernetSetMode&& msg) override;
    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;

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

} // namespace eth
} // namespace sim
} // namespace ib
