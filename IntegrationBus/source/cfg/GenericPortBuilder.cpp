// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericPortBuilder.hpp"

#include "ParticipantBuilder.hpp"
#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {

GenericPortBuilder::GenericPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _port.name = std::move(name);
    _port.endpointId = endpointId;
}

auto GenericPortBuilder::operator->() -> ParticipantBuilder*
{
    return Parent();
}

auto GenericPortBuilder::WithLink(const std::string& linkname) -> GenericPortBuilder&
{
    auto&& qualifiedName = Parent()->MakeQualifiedName(_port.name);
    auto&& link = Parent()->Parent()->AddOrGetLink(_port.linkType, linkname);
    link.AddEndpoint(qualifiedName);

    return WithLinkId(link.LinkId());
}

auto GenericPortBuilder::WithLinkId(int16_t linkId) -> GenericPortBuilder&
{
    _port.linkId = linkId;
    return *this;
}

auto GenericPortBuilder::WithEndpointId(mw::EndpointId id) -> GenericPortBuilder&
{
    _port.endpointId = id;
    return *this;
}

auto GenericPortBuilder::WithProtocolType(GenericPort::ProtocolType protocolType) -> GenericPortBuilder&
{
    _port.protocolType = protocolType;
    return *this;
}
    
auto GenericPortBuilder::WithDefinitionUri(std::string uri) -> GenericPortBuilder&
{
    _port.definitionUri = std::move(uri);
    return *this;
}

auto GenericPortBuilder::WithTraceSink(std::string sinkName) -> GenericPortBuilder&
{
    _port.useTraceSinks.emplace_back(std::move(sinkName));
    return *this;
}

auto GenericPortBuilder::Build() -> GenericPort
{
    if (_port.linkId == -1)
    {
        WithLink(_port.name);
    }
    _port.replay = _replayBuilder.Build();

    return std::move(_port);
}


auto GenericPortBuilder::WithReplay(std::string sourceName) -> ReplayBuilder&
{
    return _replayBuilder.UseTraceSource(sourceName);
}

} // namespace cfg
} // namespace ib
