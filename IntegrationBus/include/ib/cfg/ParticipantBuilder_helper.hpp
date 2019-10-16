// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <string>

namespace ib {
namespace cfg {

class LinkBuilder;

// IB Internal: resolve cyclic dependencies by not de-referencing Parent() (ie., an incomplete type at declaration point) in the children directly
template<template<class> class Builder, typename BuilderCfg>
auto ParticipantBuilder_MakeQualifiedName(Builder<BuilderCfg> &builder, const std::string controllerName) -> std::string
{
    return builder.Parent()->MakeQualifiedName(controllerName);
}
template<template<class> class Builder, typename BuilderCfg, typename LinkType>
auto ParticipantBuilder_AddOrGetLink(Builder<BuilderCfg> &builder, LinkType linkType, const std::string& linkName) -> LinkBuilder&
{
    auto *parent = builder.Parent();
    return builder->Parent()->AddOrGetLink(linkType, linkName);
}

} // namespace cfg
} // namespace ib
