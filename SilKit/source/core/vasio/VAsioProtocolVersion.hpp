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

#include <tuple>
#include <cstdint>
#include <string>
#include <ostream>

#include "VAsioDatatypes.hpp" //for RegistryMsgHeader
#include "MessageBuffer.hpp" //for ProtocolVersion

namespace SilKit {
namespace Core {

inline auto ExtractProtocolVersion(const RegistryMsgHeader& header) -> ProtocolVersion;
inline auto MakeRegistryMsgHeader(ProtocolVersion version);
inline auto MapVersionToRelease(const SilKit::Core::RegistryMsgHeader& registryMsgHeader) -> std::string;
inline constexpr auto CurrentProtocolVersion() -> ProtocolVersion;
inline bool ProtocolVersionSupported(const RegistryMsgHeader& header);

inline auto operator<<(std::ostream& out, const ProtocolVersion& header) -> std::ostream&;

//////////////////////////////////////////////////////////////////////
//  Inline Implementations
//////////////////////////////////////////////////////////////////////
auto ExtractProtocolVersion(const RegistryMsgHeader& header) -> ProtocolVersion
{
    return {header.versionHigh, header.versionLow};
}

auto MakeRegistryMsgHeader(ProtocolVersion version)
{
    RegistryMsgHeader header;
    header.versionHigh = version.major;
    header.versionLow = version.minor;
    return header;
}

//! Map ProtocolVersion ranges to SILKIT distribution releases
auto MapVersionToRelease(const SilKit::Core::RegistryMsgHeader& registryMsgHeader) -> std::string
{
    const auto version = ExtractProtocolVersion(registryMsgHeader);
    if (version.major == 1)
    {
        return {"< v2.0.0"};
    }
    else if (version == ProtocolVersion{2,0})
    {
        return {"v2.0.0 - v3.4.0"};
    }
    else if (version == ProtocolVersion{2,1})
    {
        return {"v3.4.1 - v3.99.21"};
    }
    else if (version == ProtocolVersion{3,0})
    {
        return {"v3.99.22"};
    }
    else if (version == ProtocolVersion{3,1})
    {
        return {"v3.99.23 - current"};
    }

    return {"Unknown version range"};
}

bool ProtocolVersionSupported(const RegistryMsgHeader& header)
{
    const auto version = ExtractProtocolVersion(header);
    if(version == ProtocolVersion{3, 0})
    {
        //3.99.21: bumped version to be explicitly incompatible with prior releases (MVP3, CANoe16)
        return true;
    }
    else if (version == ProtocolVersion{3,1})
    {
        //3.99.23: bumped version to test backwards compatibility with removed VAsioPeerUri in ParticipantAnnouncement
        return true;
    }
    // NB: Add your explicit backward compatibility here, ensure that Serialize/Deserialize can handle the ProtocolVersion transparently.

    return false;
 }

inline auto operator<<(std::ostream& out, const ProtocolVersion& header) -> std::ostream&
{
    out << static_cast<int>(header.major) << "." << static_cast<int>(header.minor);
    return out;
}

} // namespace Core
} // namespace SilKit
