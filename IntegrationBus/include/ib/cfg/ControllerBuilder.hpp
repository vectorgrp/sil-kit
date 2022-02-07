// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include <memory>

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"
#include "ParticipantBuilder_detail.hpp"
#include "ReplayBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

template<class ControllerCfg>
class ControllerBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    ControllerBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId);

    auto WithLink(const std::string& networkName) -> ControllerBuilder&;
    auto WithLinkId(int16_t linkId) -> ControllerBuilder&;
    auto WithEndpointId(mw::EndpointId id) -> ControllerBuilder&;
    IntegrationBusAPI auto WithMacAddress(std::string macAddress) -> ControllerBuilder&;
    IntegrationBusAPI auto WithClusterParameters(const sim::fr::ClusterParameters& clusterParameters) -> ControllerBuilder&;
    IntegrationBusAPI auto WithNodeParameters(const sim::fr::NodeParameters& nodeParameters) -> ControllerBuilder&;
    IntegrationBusAPI auto WithTxBufferConfigs(const std::vector<sim::fr::TxBufferConfig>& txBufferConfigs) -> ControllerBuilder&;

    auto WithTraceSink(std::string sinkName) -> ControllerBuilder&;
    auto WithReplay(std::string useTraceSource) -> ReplayBuilder&;
    auto operator->() -> ParticipantBuilder*;

    auto Build() -> ControllerCfg;

private:
    ControllerCfg _controller;
    std::string _link;
    std::unique_ptr<ReplayBuilder> _replayBuilder;
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
auto ControllerBuilder<ControllerCfg>::WithLink(const std::string& networkName) -> ControllerBuilder&
{
    auto&& qualifiedName = detail::ParticipantBuilder_MakeQualifiedName(*this, _controller.name);
    auto&& link = detail::ParticipantBuilder_AddOrGetLink(*this, _controller.linkType, networkName);
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
    if(_replayBuilder)
    {
        _controller.replay = _replayBuilder->Build();
    }

    return std::move(_controller);
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::WithTraceSink(std::string sinkName) -> ControllerBuilder&
{
    _controller.useTraceSinks.emplace_back(std::move(sinkName));

    return *this;
}

template<class ControllerCfg>
auto ControllerBuilder<ControllerCfg>::WithReplay(std::string sourceName) -> ReplayBuilder&
{
    _replayBuilder = std::make_unique<ReplayBuilder>(sourceName);
    return *_replayBuilder;
}

} // namespace deprecated
} // namespace cfg
} // namespace ib

