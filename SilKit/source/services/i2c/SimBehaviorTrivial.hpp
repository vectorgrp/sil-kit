// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>

#include "IMsgForI2cController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

class I2cController;

class SimBehaviorTrivial : public ISimBehavior
{
public:
    SimBehaviorTrivial(Core::IParticipantInternal* participant, I2cController* controller,
                       Services::Orchestration::ITimeProvider* timeProvider);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(WireI2cFrameEvent&& msg) override;
    void SendMsg(WireI2cControllerConfig&& msg) override;
    void OnControllerConfig(const Core::IServiceEndpoint* from, const WireI2cControllerConfig& config) override;

private:
    template <typename MsgT>
    void ReceiveMsg(const MsgT& msg);

    auto HasSlave(I2cAddress address) const -> bool;

    Core::IParticipantInternal* _participant{nullptr};
    I2cController* _parentController{nullptr};
    const Core::IServiceEndpoint* _parentServiceEndpoint{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};

    //! Registry of known slaves: address → true (presence indicator only)
    std::map<I2cAddress, bool> _slaveRegistry;
};

} // namespace I2c
} // namespace Services
} // namespace SilKit
