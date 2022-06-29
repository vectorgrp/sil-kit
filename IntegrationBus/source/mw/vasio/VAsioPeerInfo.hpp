// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <map>

#include "EndpointAddress.hpp"

namespace ib {
namespace mw {

//!< VAsioPeerInfo contains peer information such as acceptor URIs, name and ID
struct VAsioPeerInfo
{
    std::string participantName;
    ParticipantId participantId;
    std::vector<std::string> acceptorUris;
    //!< The capabilities of the peer, encoded in a string.
    //! this is left as simple to decode as possible, in case we need
    //! it to upgrade the connection to a different protocol in the future.
    std::string capabilities;
};

} // namespace mw
} // namespace ib
