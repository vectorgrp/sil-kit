// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
