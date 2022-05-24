// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <tuple>
namespace ib {
namespace mw {

// The protocol version is directly tied to the MessageBuffer for backward compatibility in Ser/Des

using ProtocolVersion = std::tuple<int, int>;

inline constexpr auto CurrentProtocolVersion() -> ProtocolVersion
{
    return {3, 1};
}

} // namespace mw
} // namespace ib
