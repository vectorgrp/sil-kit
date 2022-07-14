// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <map>
#include <chrono>
#include <string>
#include <vector>

#include <cstdint>

#include "silkit/util/Span.hpp"

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
    Util::Span<const uint8_t> argumentData;
};

using RpcCallHandler = std::function<void(IRpcServer* server, const RpcCallEvent& event)>;

struct RpcCallResultEvent
{
    std::chrono::nanoseconds timestamp;
    IRpcCallHandle* callHandle;
    RpcCallStatus callStatus;
    Util::Span<const uint8_t> resultData;
};

using RpcCallResultHandler = std::function<void(IRpcClient* client, const RpcCallResultEvent& event)>;

struct RpcDiscoveryResult
{
    std::string functionName;
    std::string mediaType;
    std::map<std::string, std::string> labels;
};

using RpcDiscoveryResultHandler = std::function<void(const std::vector<RpcDiscoveryResult>& discoveryResult)>;

} // namespace Rpc
} // namespace Services
} // namespace SilKit
