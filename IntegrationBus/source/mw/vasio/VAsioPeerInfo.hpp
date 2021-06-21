// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {

struct VAsioPeerInfo
{
    std::string participantName;
    ParticipantId participantId;
    std::string acceptorHost;
    uint16_t acceptorPort;
    std::vector<std::string> acceptorUris;
};

} // mw
} // namespace ib
