/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "MessageBuffer.hpp"
#include "WireLinMessages.hpp"

#include "silkit/services/lin/LinDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

void Serialize(SilKit::Core::MessageBuffer& buffer, const LinFrame& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireLinControllerConfig& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinFrameResponse& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinSendFrameRequest& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinSendFrameHeaderRequest& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinTransmission& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinWakeupPulse& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinControllerStatusUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const LinFrameResponseUpdate& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, LinFrame& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireLinControllerConfig& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinFrameResponse& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinSendFrameRequest& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinSendFrameHeaderRequest& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinTransmission& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinWakeupPulse& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinControllerStatusUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LinFrameResponseUpdate& out);

} // namespace Lin
} // namespace Services
} // namespace SilKit
