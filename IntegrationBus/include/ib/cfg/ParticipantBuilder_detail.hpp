// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <string>

namespace ib {
namespace cfg {

class LinkBuilder;

namespace detail {

// IB Internal: resolve cyclic dependencies by not de-referencing Parent() (ie., an incomplete type at declaration point) in the children directly
template<class Builder>
auto ParticipantBuilder_MakeQualifiedName(Builder &builder, const std::string controllerName) -> std::string
{
    return builder.Parent()->MakeQualifiedName(controllerName);
}
template<class Builder, typename LinkType>
auto ParticipantBuilder_AddOrGetLink(Builder &builder, LinkType linkType, const std::string& linkName) -> LinkBuilder&
{
    return builder->Parent()->AddOrGetLink(linkType, linkName);
}

} // namespace detail
} // namespace cfg
} // namespace ib
