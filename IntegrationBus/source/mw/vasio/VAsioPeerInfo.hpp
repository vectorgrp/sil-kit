// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <map>

#include "EndpointAddress.hpp"

namespace ib {
namespace mw {

//!< VAsioPeerInfo is kept for backward compatibility with Registry messages version 2.0
struct VAsioPeerInfo
{
    std::string participantName;
    ParticipantId participantId;
    std::string acceptorHost;
    uint16_t acceptorPort;
};
//!< Peer Info in URI format, supports multiple endpoints of tcp or local socket types.
// NB: we could not add the URIs to the VAsioPeerInfo struct directly. The PeerInfo is
//     used inside a vector, and the newly added URI member would change its size.
//     We would be unable to recognize the end of the PeerInfo and the beginning of the
//     adjacent PeerInfo in the vector.
// TODO in future versions we should unify VAsioPeerInfo and VAsioPeerUri again.
struct VAsioPeerUri
{
    std::string participantName;
    ParticipantId participantId;
    std::vector<std::string> acceptorUris;
    //!< The capabilities of the peer, encoded in a string.
    //! this is left as simple to decode as possible, in case we need
    //! it to upgrade the connection to a different protocol in the future.
    std::string capabilities;
};

} // mw
} // namespace ib
