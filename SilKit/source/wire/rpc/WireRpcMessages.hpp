// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
