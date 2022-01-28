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

/*! \brief Serialization details.
 *
 * Specification of the format used by individual Rpc Clients and Servers. Single asterisk for wildcard
 * fields
 */
struct RpcExchangeFormat
{
    std::string mimeType;
};

inline bool operator==(const RpcExchangeFormat& lhs, const RpcExchangeFormat& rhs)
{
    return lhs.mimeType == rhs.mimeType;
}

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

struct ClientAnnouncement
{
    std::string                        functionName;
    RpcExchangeFormat                  exchangeFormat;
    std::string                        clientUUID;
    std::map<std::string, std::string> labels;
};

struct ServerAcknowledge
{
    std::string clientUUID;
};

} // namespace rpc
} // namespace sim
} // namespace ib

