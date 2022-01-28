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

bool wildcardStringMatch(const std::string& s1, const std::string& s2)
{
    return s1 == "*" || s2 == "*" || s1 == s2;
}

bool Match(const RpcExchangeFormat& lhs, const RpcExchangeFormat& rhs)
{
    return wildcardStringMatch(lhs.mimeType, rhs.mimeType);
}

} // namespace rpc
} // namespace sim
} // namespace ib
