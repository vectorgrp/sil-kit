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

#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/pubsub/string_utils.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace PubSub {

struct WireDataMessageEvent
{
    std::chrono::nanoseconds timestamp;
    Util::SharedVector<uint8_t> data;
};

inline auto ToDataMessageEvent(const WireDataMessageEvent& wireDataMessageEvent) -> DataMessageEvent;
inline auto MakeWireDataMessageEvent(const DataMessageEvent& dataMessageEvent) -> WireDataMessageEvent;

inline std::string to_string(const WireDataMessageEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const WireDataMessageEvent& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

auto ToDataMessageEvent(const WireDataMessageEvent& wireDataMessageEvent) -> DataMessageEvent
{
    return {wireDataMessageEvent.timestamp, wireDataMessageEvent.data.AsSpan()};
}

auto MakeWireDataMessageEvent(const DataMessageEvent& dataMessageEvent) -> WireDataMessageEvent
{
    return {dataMessageEvent.timestamp, dataMessageEvent.data};
}

std::string to_string(const WireDataMessageEvent& msg)
{
    return to_string(ToDataMessageEvent(msg));
}

std::ostream& operator<<(std::ostream& out, const WireDataMessageEvent& msg)
{
    return out << ToDataMessageEvent(msg);
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
