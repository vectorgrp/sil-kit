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
struct VAsioPeerUri
{
    std::string participantName;
    ParticipantId participantId;
    std::vector<std::string> acceptorUris;
};

} // mw
} // namespace ib
