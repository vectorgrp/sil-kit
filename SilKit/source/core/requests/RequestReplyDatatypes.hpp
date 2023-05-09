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

#include <vector>
#include <map>
#include <sstream>

#include "Uuid.hpp"
#include "IServiceEndpoint.hpp"
#include "silkit/util/PrintableHexString.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

// Do not change the order of the FunctionType enum.
enum class FunctionType : uint16_t
{
    Invalid = 0,
    ParticipantReplies = 1
};

enum class CallReturnStatus : uint16_t
{
    Success = 0,
    UnknownFunctionType = 1,
    ProcedureError = 2,
    RecipientDisconnected = 3
};

struct RequestReplyCall
{
    Util::Uuid callUuid;
    FunctionType functionType{FunctionType::Invalid};
    std::vector<uint8_t> callData{};
};

struct RequestReplyCallReturn
{
    Util::Uuid callUuid;
    FunctionType functionType{FunctionType::Invalid};
    std::vector<uint8_t> callReturnData{};
    CallReturnStatus callReturnStatus{CallReturnStatus::Success};
};

////////////////////////////////////////////////////////////////////////////////
// Inline operators
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const RequestReplyCall& lhs, const RequestReplyCall& rhs)
{
    return lhs.callData == rhs.callData && lhs.callUuid == rhs.callUuid && lhs.functionType == rhs.functionType;
}
inline bool operator==(const RequestReplyCallReturn& lhs, const RequestReplyCallReturn& rhs)
{
    return lhs.callReturnData == rhs.callReturnData && lhs.callUuid == rhs.callUuid
           && lhs.functionType == rhs.functionType && lhs.callReturnStatus == rhs.callReturnStatus;
}

////////////////////////////////////////////////////////////////////////////////
// Inline string utils
////////////////////////////////////////////////////////////////////////////////

inline std::string to_string(FunctionType value)
{
    switch (value)
    {
    case FunctionType::ParticipantReplies: return "ParticipantReplies";
    case FunctionType::Invalid: return "Invalid";
    };
    throw SilKit::TypeConversionError{};
}

inline std::string to_string(CallReturnStatus value)
{
    switch (value)
    {
    case CallReturnStatus::Success: return "Success";
    case CallReturnStatus::UnknownFunctionType: return "UnknownFunctionType";
    case CallReturnStatus::ProcedureError: return "ProcedureError";
    case CallReturnStatus::RecipientDisconnected: return "RecipientDisconnected";
    };
    throw SilKit::TypeConversionError{};
}

inline std::ostream& operator<<(std::ostream& out, const RequestReplyCall& msg)
{
    out << "RequestReplyCall{functionType=" << to_string(msg.functionType) 
        << ", callUuid=" << msg.callUuid
        << ", data=" << Util::AsHexString(msg.callData).WithSeparator(" ").WithMaxLength(16)
        << ", size=" << msg.callData.size() << "}";

    return out;
}

inline std::ostream& operator<<(std::ostream& out, const RequestReplyCallReturn& msg)
{
    out << "RequestReplyCallReturn{functionType=" << to_string(msg.functionType) << ", callUuid=" << msg.callUuid
        << ", data=" << Util::AsHexString(msg.callReturnData).WithSeparator(" ").WithMaxLength(16)
        << ", size=" << msg.callReturnData.size() << ", callReturnStatus=" << to_string(msg.callReturnStatus) << "}";

    return out;
}

inline std::string to_string(const RequestReplyCall& msg)
{
    std::stringstream str;
    str << msg;
    return str.str();
}

inline std::string to_string(const RequestReplyCallReturn& msg)
{
    std::stringstream str;
    str << msg;
    return str.str();
}

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
