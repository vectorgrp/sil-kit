// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <sstream>

#include "silkit/util/PrintableHexString.hpp"

#include "PubSubDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

inline std::string to_string(const DataMessageEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const DataMessageEvent& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const DataMessageEvent& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const DataMessageEvent& msg)
{
    return out << "PubSub::DataMessageEvent{data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
