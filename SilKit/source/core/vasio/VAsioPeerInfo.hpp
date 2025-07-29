// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <map>

#include "EndpointAddress.hpp"

namespace SilKit {
namespace Core {

//! VAsioPeerInfo contains peer information such as acceptor URIs, name and ID
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

inline bool operator<(const VAsioPeerInfo& lhs, const VAsioPeerInfo& rhs)
{
    return lhs.participantName < rhs.participantName;
}

} // namespace Core
} // namespace SilKit
