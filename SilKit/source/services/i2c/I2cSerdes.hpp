// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "WireI2cMessages.hpp"

#include "silkit/services/i2c/I2cDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireI2cFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const I2cAcknowledge& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireI2cControllerConfig& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, WireI2cFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, I2cAcknowledge& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireI2cControllerConfig& out);

} // namespace I2c
} // namespace Services
} // namespace SilKit
