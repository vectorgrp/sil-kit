// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "ib/mw/EndpointAddress.hpp"

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
};

} // mw
} // namespace ib
