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

#include "silkit/services/can/CanDatatypes.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(Core::IParticipantInternal* participant, CanController* canController,
                Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(CanConfigureBaudrate&& msg) override;
    void SendMsg(CanSetControllerMode&& msg) override;
    void SendMsg(WireCanFrameEvent&& msg) override;

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

} // namespace Can
} // namespace Services
} // namespace SilKit
