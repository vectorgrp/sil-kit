// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
