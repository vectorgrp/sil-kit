// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <map>

namespace ib {
namespace sim {
namespace rpc {

class IRpcClient;
class IRpcServer;
class IRpcCallHandle;

enum class CallStatus : uint8_t
{
    Success,
    ServerNotReachable,
    UndefinedError
};

using CallReturnHandler =
    std::function<void(ib::sim::rpc::IRpcClient* client, ib::sim::rpc::IRpcCallHandle* callHandle, const ib::sim::rpc::CallStatus callStatus,
                       const std::vector<uint8_t>& returnData)>;

using CallProcessor = std::function<void(ib::sim::rpc::IRpcServer* server, ib::sim::rpc::IRpcCallHandle* callHandle,
                                         const std::vector<uint8_t>& argumentData)>;

struct RpcDiscoveryResult
{
    std::string rpcChannel;
    std::string mediaType;
    std::map<std::string, std::string> labels;
};

using DiscoveryResultHandler = std::function<void(const std::vector<RpcDiscoveryResult>& discoveryResults)>;

// IbMessages
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
    CallUUID callUUID;
    std::vector<uint8_t> data;
};

/*! \brief Rpc response with function return data
 *
 * Rpcs run over an abstract channel, without timing effects and/or data type constraints
 */
struct FunctionCallResponse
{
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

} // namespace rpc
} // namespace sim
} // namespace ib

