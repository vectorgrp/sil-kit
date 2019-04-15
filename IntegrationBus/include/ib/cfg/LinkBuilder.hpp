// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {

class LinkBuilder
{
public:
    IntegrationBusAPI LinkBuilder(Link::Type linkType, std::string name, int16_t linkId);

    IntegrationBusAPI auto AddEndpoint(std::string qualifiedName) -> LinkBuilder&;

    IntegrationBusAPI auto Build() -> Link;

    IntegrationBusAPI inline auto Name() const -> const std::string&;
    IntegrationBusAPI inline auto LinkId() const -> int16_t;
    IntegrationBusAPI inline auto LinkType() const -> Link::Type;

private:
    Link link;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
inline auto LinkBuilder::Name() const -> const std::string&
{
    return link.name;
}

inline auto LinkBuilder::LinkId() const -> int16_t
{
    return link.id;
}

inline auto LinkBuilder::LinkType() const -> Link::Type
{
    return link.type;
}

} // namespace cfg
} // namespace ib
