// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

inline std::string to_string(const GenericMessage& msg);

inline std::ostream& operator<<(std::ostream& out, const GenericMessage& msg);
    

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(const GenericMessage& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const GenericMessage& msg)
{
    return out << "generic::GenericMessage{data="
               << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size()
               << "}";
}

} // namespace generic
} // namespace sim
} // namespace ib
