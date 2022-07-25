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
#include "WireFlexrayMessages.hpp"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayCycleStartEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayHostCommand& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayControllerConfig& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayTxBufferUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayPocStatusEvent& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayCycleStartEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayHostCommand& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayControllerConfig& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayTxBufferUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayPocStatusEvent& out);

} // namespace Flexray    
} // namespace Services
} // namespace SilKit
