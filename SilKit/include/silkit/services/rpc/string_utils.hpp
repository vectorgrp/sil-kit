/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
