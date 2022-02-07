// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataPortBuilder.hpp"

#include <algorithm>

#include "ParticipantBuilder.hpp"
#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

DataPortBuilder::DataPortBuilder(ParticipantBuilder *participant, std::string name, mw::EndpointId endpointId)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _port.name = std::move(name);
    _port.endpointId = endpointId;
}

auto DataPortBuilder::operator->() -> ParticipantBuilder*
{
    return Parent();
}

auto DataPortBuilder::WithLink(const std::string& networkName) -> DataPortBuilder&
{
    auto&& qualifiedName = Parent()->MakeQualifiedName(_port.name);
    auto&& link = Parent()->Parent()->AddOrGetLink(_port.linkType, networkName);
    link.AddEndpoint(qualifiedName);

    return WithLinkId(link.LinkId());
}

auto DataPortBuilder::WithLinkId(int16_t linkId) -> DataPortBuilder&
{
    _port.linkId = linkId;
    return *this;
}

auto DataPortBuilder::WithEndpointId(mw::EndpointId id) -> DataPortBuilder&
{
    _port.endpointId = id;
    return *this;
}

auto DataPortBuilder::WithTraceSink(std::string sinkName) -> DataPortBuilder&
{
    _port.useTraceSinks.emplace_back(std::move(sinkName));
    return *this;
}

auto DataPortBuilder::Build() -> DataPort
{
    if (_port.linkId == -1)
    {
        WithLink(_port.name);
    }
    if (_replayBuilder)
    {
        _port.replay = _replayBuilder->Build();
    }

    DataPort newConfig{};
    std::swap(_port, newConfig);
    return newConfig;
}


auto DataPortBuilder::WithReplay(std::string sourceName) -> ReplayBuilder&
{
    _replayBuilder = std::make_unique<ReplayBuilder>(sourceName);
    return *_replayBuilder;
}

} // inline namespace deprecated
} // namespace cfg
} // namespace ib
