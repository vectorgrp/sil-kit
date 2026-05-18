// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForI2cController.hpp"
#include "IParticipantInternal.hpp"

#include "ISimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

class I2cController;

class SimBehaviorDetailed : public ISimBehavior
{
public:
    SimBehaviorDetailed(Core::IParticipantInternal* participant, I2cController* controller,
                        const Core::ServiceDescriptor& serviceDescriptor);

    auto AllowReception(const Core::IServiceEndpoint* from) const -> bool override;
    void SendMsg(WireI2cFrameEvent&& msg) override;
    void SendMsg(WireI2cControllerConfig&& msg) override;
    void OnControllerConfig(const Core::IServiceEndpoint* from, const WireI2cControllerConfig& config) override;

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

} // namespace I2c
} // namespace Services
} // namespace SilKit
