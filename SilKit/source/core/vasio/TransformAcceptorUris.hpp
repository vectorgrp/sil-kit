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

#include "silkit/services/logging/ILogger.hpp"

#include "IVAsioPeer.hpp"
#include "Uri.hpp"

#include <string>
#include <vector>

namespace SilKit {
namespace Core {

struct UriInfo
{
    //! \brief The URI describes a local endpoint, either local-domain, or a loopback adddress.
    bool local{false};
    bool ip4{false};
    bool ip6{false};
    bool catchallIp{false};
    bool loopbackIp{false};
};

auto GetUriInfo(const Uri& uri) -> UriInfo;

auto TransformAcceptorUris(SilKit::Services::Logging::ILogger* logger, IVAsioPeer* advertisedPeer,
                           IVAsioPeer* audiencePeer) -> std::vector<std::string>;

} // namespace Core
} // namespace SilKit
