// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <string>
#include <cassert>

namespace ib {
namespace cfg {
inline namespace deprecated {

class LinkBuilder;

namespace detail {

// IB Internal: resolve cyclic dependencies by not de-referencing Parent() (ie., an incomplete type at declaration point) in the children directly
template<class Builder>
auto ParticipantBuilder_MakeQualifiedName(Builder &builder, const std::string controllerName) -> std::string
{
    auto* participantBuilder = builder.Parent();
    assert(participantBuilder != nullptr);
    return participantBuilder->MakeQualifiedName(controllerName);
}
template<class Builder, typename LinkType>
auto ParticipantBuilder_AddOrGetLink(Builder& builder, LinkType linkType, const std::string& networkName) -> LinkBuilder&
{
    auto* participantBuilder = builder.Parent();
    assert(participantBuilder != nullptr);
    auto* simBuilder = participantBuilder->Parent();
    assert(simBuilder != nullptr);
    return simBuilder->AddOrGetLink(linkType, networkName);
}

} // namespace detail
} // namespace deprecated
} // namespace cfg
} // namespace ib
