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

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

#include "WireFlexrayMessages.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

bool operator==(const FlexrayHeader& lhs, const FlexrayHeader& rhs);
bool operator==(const FlexrayFrame& lhs, const FlexrayFrame& rhs);
bool operator==(const FlexrayFrameEvent& lhs, const FlexrayFrameEvent& rhs);
bool operator==(const FlexrayFrameTransmitEvent& lhs, const FlexrayFrameTransmitEvent& rhs);
bool operator==(const FlexraySymbolEvent& lhs, const FlexraySymbolEvent& rhs);
bool operator==(const FlexrayWakeupEvent& lhs, const FlexrayWakeupEvent& rhs);
bool operator==(const FlexrayTxBufferConfigUpdate& lhs, const FlexrayTxBufferConfigUpdate& rhs);
bool operator==(const WireFlexrayTxBufferUpdate& lhs, const WireFlexrayTxBufferUpdate& rhs);
bool operator==(const FlexrayControllerConfig& lhs, const FlexrayControllerConfig& rhs);
bool operator==(const FlexrayHostCommand& lhs, const FlexrayHostCommand& rhs);
bool operator==(const FlexrayPocStatusEvent& lhs, const FlexrayPocStatusEvent& rhs);
bool operator==(const FlexrayCycleStartEvent& lhs, const FlexrayCycleStartEvent& rhs);

} // namespace Flexray
} // namespace Services
} // namespace SilKit
