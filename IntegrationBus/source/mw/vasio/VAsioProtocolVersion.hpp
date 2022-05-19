
// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <cstdint>
#include <string>

#include "VAsioDatatypes.hpp" //for RegistryMsgHeader

namespace ib {
namespace mw {
using ProtocolVersion = std::tuple<int, int>;


inline auto from_header(const RegistryMsgHeader& header) -> ProtocolVersion;
inline auto to_header(ProtocolVersion version);
inline auto MapVersionToRelease(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string;
inline constexpr auto CurrentProtocolVersion() -> ProtocolVersion;

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

constexpr auto CurrentProtocolVersion() -> ProtocolVersion
{
    //VS2015 does not support proper c++14:
    //const RegistryMsgHeader header;
    //return {header.versionHigh, header.versionLow};
    return {RegistryMsgHeader{}.versionHigh,
        RegistryMsgHeader{}.versionLow};
}

} // mw
} // namespace ib
