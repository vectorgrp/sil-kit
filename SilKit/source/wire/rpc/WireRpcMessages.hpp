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

#include "silkit/services/rpc/RpcDatatypes.hpp"
#include "silkit/services/rpc/string_utils.hpp"
#include "silkit/util/PrintableHexString.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Rpc {

struct CallUUID
{
    uint64_t ab;
    uint64_t cd;
};

/*! \brief Rpc with function parameter data
 *
 * Rpcs run over an abstract channel, without timing effects and/or data type constraints
 */
struct FunctionCall
{
    std::chrono::nanoseconds timestamp;
    CallUUID callUUID;
    std::vector<uint8_t> data;
};

/*! \brief Rpc response with function return data
 *
 * Rpcs run over an abstract channel, without timing effects and/or data type constraints
 */
struct FunctionCallResponse
{
    std::chrono::nanoseconds timestamp;
    CallUUID callUUID;
    std::vector<uint8_t> data;
};

inline bool operator==(const CallUUID& lhs, const CallUUID& rhs);
inline bool operator==(const FunctionCall& lhs, const FunctionCall& rhs);
inline bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs);

inline std::string to_string(const CallUUID& msg);
inline std::ostream& operator<<(std::ostream& out, const CallUUID& msg);

inline std::string to_string(const FunctionCall& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCall& msg);

inline std::string to_string(const FunctionCallResponse& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

bool operator==(const CallUUID& lhs, const CallUUID& rhs)
{
    return lhs.ab == rhs.ab && lhs.cd == rhs.cd;
}

bool operator==(const FunctionCall& lhs, const FunctionCall& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

std::string to_string(const CallUUID& callUUID)
{
    std::stringstream out;
    out << callUUID.ab << callUUID.cd;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const CallUUID& msg)
{
    return out << to_string(msg);
}

std::string to_string(const FunctionCall& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const FunctionCall& msg)
{
    return out << "rpc::FunctionCall{callUUID=" << msg.callUUID
               << ", data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}

std::string to_string(const FunctionCallResponse& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg)
{
    return out << "rpc::FunctionCallResponse{callUUID=" << msg.callUUID
               << ", data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
