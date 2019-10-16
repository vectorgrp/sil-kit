// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"
#include "ParticipantBuilder_Helper.hpp"

namespace ib {
namespace cfg {

template<class ControllerCfg>
class ControllerBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    ControllerBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId);

    auto WithLink(const std::string& linkname) -> ControllerBuilder&;
    auto WithLinkId(int16_t linkId) -> ControllerBuilder&;
    auto WithEndpointId(mw::EndpointId id) -> ControllerBuilder&;
    IntegrationBusAPI auto WithMacAddress(std::string macAddress) -> ControllerBuilder&;
    auto WithClusterParameters(const sim::fr::ClusterParameters& clusterParameters) -> ControllerBuilder&;
    auto WithNodeParameters(const sim::fr::NodeParameters& nodeParameters) -> ControllerBuilder&;
    auto WithTxBufferConfigs(const std::vector<sim::fr::TxBufferConfig>& txBufferConfigs) -> ControllerBuilder&;

    auto operator->() -> ParticipantBuilder*;

    auto Build() -> ControllerCfg;

private:
    ControllerCfg _controller;
    std::string _link;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<class ControllerCfg>
ControllerBuilder<ControllerCfg>::ControllerBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _controller.name = std::move(name);
    _controller.endpointId = endpointId;
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::operator->() -> ParticipantBuilder*
{
    return Parent();
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::WithLink(const std::string& linkname) -> ControllerBuilder&
{
    auto&& qualifiedName = ParticipantBuilder_MakeQualifiedName(*this, _controller.name);
    auto&& link = ParticipantBuilder_AddOrGetLink(*this, _controller.linkType, linkname);
    link.AddEndpoint(qualifiedName);

    return WithLinkId(link.LinkId());
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::WithLinkId(int16_t linkId) -> ControllerBuilder&
{
    _controller.linkId = linkId;
    return *this;
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::WithEndpointId(mw::EndpointId id) -> ControllerBuilder&
{
    _controller.endpointId = id;
    return *this;
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::Build() -> ControllerCfg
{
    if (_controller.linkId == -1)
    {
        WithLink(_controller.name);
    }
    return std::move(_controller);
}

} // namespace cfg
} // namespace ib

