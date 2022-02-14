// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace rpc {

bool inline operator==(const CallUUID& lhs, const CallUUID& rhs)
{
    return lhs.ab == rhs.ab && lhs.cd == rhs.cd;
}
bool inline operator!=(const CallUUID& lhs, const CallUUID& rhs)
{
    return lhs.ab != rhs.ab || lhs.cd != rhs.cd;
}

bool operator==(const FunctionCall& lhs, const FunctionCall& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs)
{
    return lhs.callUUID == rhs.callUUID && lhs.data == rhs.data;
}

bool Match(const RpcExchangeFormat& clientRxf, const RpcExchangeFormat& serverRxf)
{
    return clientRxf.mediaType == "" || clientRxf.mediaType == serverRxf.mediaType;
}

bool MatchLabels(const std::map<std::string, std::string>& clientLabels,
                 const std::map<std::string, std::string>& serverLabels)
{
    if (clientLabels.size() == 0)
        return true; // clientLabels empty -> match

    if (clientLabels.size() > serverLabels.size())
        return false; // clientLabels more labels than outer set -> no match

    for (auto&& kv : clientLabels)
    {
        auto it = serverLabels.find(kv.first);
        if (it == serverLabels.end() || // Key not found -> no match
            (kv.second != "" && kv.second != (*it).second)) // Value does not match (and no wildcard given) -> no match
        {
            return false;
        }
    }
    return true; // All of clientLabels are there -> match
}

} // namespace rpc
} // namespace sim
} // namespace ib
