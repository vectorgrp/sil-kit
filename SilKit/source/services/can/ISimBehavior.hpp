// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/can/CanDatatypes.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const Core::IServiceEndpoint* from) const -> bool = 0;
    virtual void SendMsg(CanConfigureBaudrate&& msg) = 0;
    virtual void SendMsg(CanSetControllerMode&& msg) = 0;
    virtual void SendMsg(WireCanFrameEvent&& msg) = 0;
};

} // namespace Can
} // namespace Services
} // namespace SilKit
