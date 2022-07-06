// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "silkit/exception.hpp"
#include "silkit/util/PrintableHexString.hpp"

#include "RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

// RpcSilKitMessages
inline std::string   to_string(const CallUUID& msg);
inline std::ostream& operator<<(std::ostream& out, const CallUUID& msg);

inline std::string   to_string(const FunctionCall& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCall& msg);

inline std::string   to_string(const FunctionCallResponse& msg);
inline std::ostream& operator<<(std::ostream& out, const FunctionCallResponse& msg);

// RpcDatatypes
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
    return out << "Rpc::FunctionCall{callUUID=" << msg.callUUID << ", data="
               << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
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
    return out << "Rpc::FunctionCallResponse{callUUID=" << msg.callUUID
               << ", data=" << Util::AsHexString(msg.data).WithSeparator(" ").WithMaxLength(16)
               << ", size=" << msg.data.size() << "}";
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
    return out << "Rpc::RpcDiscoveryResult{"
               << "functionName=\"" << discoveryResult.functionName << "\", mediaType=" << discoveryResult.mediaType
               << ", labels=" << discoveryResult.labels
               << "}";
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
