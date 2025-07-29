// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>
#include <vector>
#include <map>

#include "SimBehaviorDetailed.hpp"
#include "SimBehaviorTrivial.hpp"

#include "silkit/services/lin/LinDatatypes.hpp"
#include "IServiceEndpoint.hpp"
#include "WireLinMessages.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController;

class SimBehavior : public ISimBehavior
{
public:
    SimBehavior(Core::IParticipantInternal* participant, LinController* linController,
                Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;

    void SendMsg(LinSendFrameRequest&& sendFrameRequest) override;
    void SendMsg(LinTransmission&& transmission) override;
    void SendMsg(WireLinControllerConfig&& controllerConfig) override;
    void SendMsg(LinSendFrameHeaderRequest&& header) override;
    void SendMsg(LinFrameResponseUpdate&& frameResponseUpdate) override;
    void SendMsg(LinControllerStatusUpdate&& statusUpdate) override;

    void ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& header) override;

    void UpdateTxBuffer(const LinFrame& frame) override;

    void GoToSleep() override;
    void Wakeup() override;
    auto CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus override;

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

} // namespace Lin
} // namespace Services
} // namespace SilKit
