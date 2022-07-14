// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <sstream>

#include "RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

// RpcDatatypes
inline std::ostream& operator<<(std::ostream& out, const std::map<std::string, std::string>& labels);

inline std::string to_string(const RpcDiscoveryResult& discoveryResult);
inline std::ostream& operator<<(std::ostream& out, const RpcDiscoveryResult& discoveryResult);


// ================================================================================
//  Inline Implementations
// ================================================================================

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
