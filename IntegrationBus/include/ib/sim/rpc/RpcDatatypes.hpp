// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <functional>
#include <map>
#include <chrono>
#include <string>

#include <cstdint>

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcClient;
class IRpcServer;
class IRpcCallHandle;

enum class RpcCallStatus : uint8_t
{
    Success,
    ServerNotReachable,
    UndefinedError
};

struct RpcCallEvent
{
    std::chrono::nanoseconds timestamp;
    IRpcCallHandle* callHandle;
    std::vector<uint8_t> argumentData;
};

using RpcCallHandler = std::function<void(IRpcServer* server, const RpcCallEvent& event)>;

struct RpcCallResultEvent
{
    std::chrono::nanoseconds timestamp;
    IRpcCallHandle* callHandle;
    RpcCallStatus callStatus;
    std::vector<uint8_t> resultData;
};

using RpcCallResultHandler = std::function<void(IRpcClient* client, const RpcCallResultEvent& event)>;

struct RpcDiscoveryResult
{
    std::string functionName;
    std::string mediaType;
    std::map<std::string, std::string> labels;
};

using RpcDiscoveryResultHandler = std::function<void(const std::vector<RpcDiscoveryResult>& discoveryResults)>;

// SilKitMessages
//-----------

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

// ================================================================================
//  Inline Implementations
// ================================================================================
inline bool operator==(const CallUUID& lhs, const CallUUID& rhs)
{
    return lhs.ab == rhs.ab && lhs.cd == rhs.cd;
}

inline bool operator==(const FunctionCall& lhs, const FunctionCall& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

inline bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
