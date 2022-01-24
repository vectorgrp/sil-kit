// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcPortBuilder.hpp"

#include <algorithm>

#include "ParticipantBuilder.hpp"
#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {

RpcPortBuilder::RpcPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _port.name = std::move(name);
    _port.endpointId = endpointId;
}

auto RpcPortBuilder::operator->() -> ParticipantBuilder*
{
    return Parent();
}

auto RpcPortBuilder::WithLink(const std::string& linkname) -> RpcPortBuilder&
{
    auto&& qualifiedName = Parent()->MakeQualifiedName(_port.name);
    auto&& link = Parent()->Parent()->AddOrGetLink(_port.linkType, linkname);
    link.AddEndpoint(qualifiedName);

    return WithLinkId(link.LinkId());
}

auto RpcPortBuilder::WithLinkId(int16_t linkId) -> RpcPortBuilder&
{
    _port.linkId = linkId;
    return *this;
}

auto RpcPortBuilder::WithEndpointId(mw::EndpointId id) -> RpcPortBuilder&
{
    _port.endpointId = id;
    return *this;
}

auto RpcPortBuilder::WithTraceSink(std::string sinkName) -> RpcPortBuilder&
{
    _port.useTraceSinks.emplace_back(std::move(sinkName));
    return *this;
}

auto RpcPortBuilder::Build() -> RpcPort
{
    if (_port.linkId == -1)
    {
        WithLink(_port.name);
    }
    if (_replayBuilder)
    {
        _port.replay = _replayBuilder->Build();
    }

    RpcPort newConfig{};
    std::swap(_port, newConfig);
    return newConfig;
}


auto RpcPortBuilder::WithReplay(std::string sourceName) -> ReplayBuilder&
{
    _replayBuilder = std::make_unique<ReplayBuilder>(sourceName);
    return *_replayBuilder;
}

} // namespace cfg
} // namespace ib
