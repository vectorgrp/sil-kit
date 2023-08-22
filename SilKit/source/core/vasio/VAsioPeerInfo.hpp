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
