// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "SimBehaviorDetailed.hpp"
#include "SimBehaviorTrivial.hpp"

#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class EthController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(Core::IParticipantInternal* participant, EthController* ethController,
                Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(WireEthernetFrameEvent&& msg) override;
    void SendMsg(EthernetSetMode&& msg) override;
    void OnReceiveAck(const EthernetFrameTransmitEvent& msg) override;

    void SetDetailedBehavior(const Core::ServiceDescriptor& simulatedLink);
    void SetTrivialBehavior();

    auto IsTrivial() const -> bool;
    auto IsDetailed() const -> bool;

private:
    template <typename MsgT>
    void SendMsgImpl(MsgT&& msg);

    SimBehaviorTrivial _trivial;
    SimBehaviorDetailed _detailed;
    ISimBehavior* _currentBehavior;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
