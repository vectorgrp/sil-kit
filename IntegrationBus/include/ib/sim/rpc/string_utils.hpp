// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "ib/exception.hpp"
#include "ib/util/PrintableHexString.hpp"

#include "RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

// RpcIbMessages
inline std::string   to_string(const CallUUID& msg);
inline std::ostream& operator<<(std::ostream& out, const CallUUID& msg);

inline std::string   to_string(const FunctionCall& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCall& msg);

inline std::string   to_string(const FunctionCallResponse& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg);

// RpcDatatypes
inline std::string   to_string(const RpcExchangeFormat& dataExchangeFormat);
inline std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& dataExchangeFormat);

inline std::ostream& operator<<(std::ostream& out, const std::map<std::string, std::string>& labels);

inline std::string to_string(const RpcDiscoveryResult& discoveryResult);
inline std::ostream& operator<<(std::ostream& out, const RpcDiscoveryResult& discoveryResult);


// ================================================================================
//  Inline Implementations
// ================================================================================

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
    return out << "rpc::FunctionCall{callUUID=" << msg.callUUID << ", data="
               << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size()
               << "}";
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
               << ", data=" << util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
}

std::string to_string(const RpcExchangeFormat& rpcExchangeFormat)
{
    std::stringstream out;
    out << rpcExchangeFormat;
    return out.str();
}
std::ostream& operator<<(std::ostream& out, const RpcExchangeFormat& rpcExchangeFormat)
{
    return out << "rpc::RpcExchangeFormat{"
               << "mediaType=\"" << rpcExchangeFormat.mediaType << "\"}";
}

std::string to_string(const RpcDiscoveryResult& discoveryResult)
{
    std::stringstream out;
    out << discoveryResult;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const std::map<std::string, std::string>& labels)
{
    out << "{";
    for (auto&& kvp : labels)
        out << "{\"" << kvp.first << "\", \"" << kvp.second << "\"}";
    out << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const RpcDiscoveryResult& discoveryResult)
{
    return out << "rpc::RpcDiscoveryResult{"
               << "functionName=\"" << discoveryResult.functionName
               << "\", exchangeFormat=" << discoveryResult.exchangeFormat 
               << ", labels=" << discoveryResult.labels
               << "}";
}

} // namespace rpc
} // namespace sim
} // namespace ib
