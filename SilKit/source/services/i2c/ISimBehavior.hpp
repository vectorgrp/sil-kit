// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/i2c/I2cDatatypes.hpp"
#include "IServiceEndpoint.hpp"
#include "WireI2cMessages.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

class ISimBehavior
{
public:
    virtual ~ISimBehavior() = default;
    virtual auto AllowReception(const Core::IServiceEndpoint* from) const -> bool = 0;
    virtual void SendMsg(WireI2cFrameEvent&& msg) = 0;
    virtual void SendMsg(WireI2cControllerConfig&& msg) = 0;
    //! Called when a WireI2cControllerConfig is received from another participant (for slave registry updates).
    virtual void OnControllerConfig(const Core::IServiceEndpoint* from, const WireI2cControllerConfig& config) = 0;
};

} // namespace I2c
} // namespace Services
} // namespace SilKit
