// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

inline std::string to_string(const DataMessage& msg);
inline std::ostream& operator<<(std::ostream& out, const DataMessage& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(const DataMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const DataMessage& msg)
{
    return out << "data::DataMessage{data="
               << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size()
               << "}";
}

} // namespace data
} // namespace sim
} // namespace ib
