// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinkBuilder.hpp"

#include <algorithm>

namespace ib {
namespace cfg {

LinkBuilder::LinkBuilder(Link::Type linkType, std::string name, int16_t linkId)
{
    link.type = linkType;
    link.name = std::move(name);
    link.id = linkId;
}

auto LinkBuilder::AddEndpoint(std::string qualifiedName) -> LinkBuilder&
{
    link.endpoints.emplace_back(std::move(qualifiedName));
    return *this;
}

auto LinkBuilder::Build() -> Link
{
    Link newConfig{};
    std::swap(link, newConfig);
    return newConfig;
}

} // namespace cfg
} // namespace ib
