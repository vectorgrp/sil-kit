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
#include "Uuid.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Rpc {

/*! \brief Rpc with function parameter data
 *
 * Rpcs run over an abstract channel, without timing effects and/or data type constraints
 */
struct FunctionCall
{
    std::chrono::nanoseconds timestamp;
    Util::Uuid callUuid;
    std::vector<uint8_t> data;
};

/*! \brief Rpc response with function return data
 *
 * Rpcs run over an abstract channel, without timing effects and/or data type constraints
 */
struct FunctionCallResponse
{
    enum struct Status : uint32_t
    {
        Success = 0,
        InternalError = 1,
    };

    std::chrono::nanoseconds timestamp;
    Util::Uuid callUuid;
    std::vector<uint8_t> data;
    Status status;
};

inline bool operator==(const FunctionCall& lhs, const FunctionCall& rhs);
inline bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs);

inline std::string to_string(const FunctionCall& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCall& msg);

inline std::string to_string(const FunctionCallResponse::Status& status);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse::Status& status);

inline std::string to_string(const FunctionCallResponse& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg);

// ================================================================================
//  Inline Implementations
// ================================================================================

bool operator==(const FunctionCall& lhs, const FunctionCall& rhs)
{
    return lhs.callUuid == rhs.callUuid && lhs.data == rhs.data;
}

bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs)
{
    return lhs.callUuid == rhs.callUuid && lhs.data == rhs.data && lhs.status == rhs.status;
}

std::string to_string(const FunctionCall& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const FunctionCall& msg)
{
    return out << "rpc::FunctionCall{callUUID=" << msg.callUuid
               << ", data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}

std::string to_string(const FunctionCallResponse::Status& status)
{
    std::ostringstream ss;
    ss << status;
    return ss.str();
}

std::ostream& operator<<(std::ostream& out, const FunctionCallResponse::Status& status)
{
    switch (status)
    {
    case FunctionCallResponse::Status::Success: return out << "FunctionCallResponse::Status::Success";
    case FunctionCallResponse::Status::InternalError: return out << "FunctionCallResponse::Status::InternalError";
    }

    return out << "FunctionCallResponse::Status("
               << static_cast<std::underlying_type_t<FunctionCallResponse::Status>>(status) << ")";
}

std::string to_string(const FunctionCallResponse& msg)
{
    std::stringstream out;
    out << msg;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg)
{
    return out << "rpc::FunctionCallResponse{callUUID=" << msg.callUuid
               << ", data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size()
               << ", status=" << msg.status << "}";
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
