
// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <cstdint>
#include <string>

#include "VAsioDatatypes.hpp" //for RegistryMsgHeader
#include "MessageBuffer.hpp" //for ProtocolVersion

namespace ib {
namespace mw {


inline auto from_header(const RegistryMsgHeader& header) -> ProtocolVersion;
inline auto to_header(ProtocolVersion version);
inline auto MapVersionToRelease(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string;
inline constexpr auto CurrentProtocolVersion() -> ProtocolVersion;
inline bool ProtocolVersionSupported(const RegistryMsgHeader& header);

//////////////////////////////////////////////////////////////////////
//  Inline Implementations
//////////////////////////////////////////////////////////////////////
auto from_header(const RegistryMsgHeader& header) -> ProtocolVersion
{
	return {header.versionHigh, header.versionLow};
}

auto to_header(ProtocolVersion version)
{
	RegistryMsgHeader header;
	header.versionHigh = static_cast<decltype(header.versionHigh)>(std::get<0>(version));
	header.versionLow = static_cast<decltype(header.versionLow)>(std::get<1>(version));
	return header;
}

//! Map ProtocolVersion ranges to VIB distribution releases
auto MapVersionToRelease(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string
{
     const auto version = from_header(registryMsgHeader);
    if (std::get<0>(version) == 1)
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
    const auto version = from_header(header);
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

} // mw
} // namespace ib
