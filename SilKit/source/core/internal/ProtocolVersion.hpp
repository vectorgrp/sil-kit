// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <tuple>
namespace SilKit {
namespace Core {

// The protocol version is directly tied to the MessageBuffer for backward compatibility in Ser/Des

struct ProtocolVersion
{
    uint16_t major;
    uint16_t minor;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

inline constexpr auto CurrentProtocolVersion() -> ProtocolVersion
{
    return {3, 1};
}

inline bool operator==(const ProtocolVersion& lhs, const ProtocolVersion& rhs)
{
    return lhs.major == rhs.major && lhs.minor == rhs.minor;
}

inline bool operator!=(const ProtocolVersion& lhs, const ProtocolVersion& rhs)
{
    return !(lhs == rhs);
}

inline bool operator<(const ProtocolVersion& lhs, const ProtocolVersion& rhs)
{
    return (lhs.major < rhs.major) || (lhs.major == rhs.major && lhs.minor < rhs.minor);
}

inline bool operator>=(const ProtocolVersion& lhs, const ProtocolVersion& rhs)
{
    return !(lhs < rhs);
}

} // namespace Core
} // namespace SilKit
