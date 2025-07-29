// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "WireCanMessages.hpp"

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

void Serialize(SilKit::Core::MessageBuffer& buffer, const Services::Can::WireCanFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const Services::Can::CanFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const Services::Can::CanControllerStatus& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const Services::Can::CanConfigureBaudrate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const Services::Can::CanSetControllerMode& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::WireCanFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanControllerStatus& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanConfigureBaudrate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanSetControllerMode& out);

} // namespace Can
} // namespace Services
} // namespace SilKit
