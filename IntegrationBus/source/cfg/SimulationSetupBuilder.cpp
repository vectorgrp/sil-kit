// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SimulationSetupBuilder.hpp"

#include <algorithm>

namespace ib {
namespace cfg {


auto SimulationSetupBuilder::Build() -> SimulationSetup
{
    auto buildConfigs =
        [](auto&& configVector, auto&& builderVector)
        {
            for (auto&& builder : builderVector)
                configVector.emplace_back(builder->Build());
        };

    buildConfigs(_config.participants, _participants);
    buildConfigs(_config.switches, _switches);
    buildConfigs(_config.networkSimulators, _networkSimulators);
    buildConfigs(_config.links, _links);

    if (_timeSync)
        _config.timeSync = _timeSync->Build();

    return std::move(_config);
}

auto SimulationSetupBuilder::AddOrGetLink(Link::Type linkType, const std::string& name) -> LinkBuilder&
{
    auto&& linkIter = std::find_if(_links.begin(), _links.end(),
        [&name](auto&& linkBuilder) { return linkBuilder->Name() == name; }
    );

    if (linkIter != _links.end())
    {
        if ((*linkIter)->LinkType() != linkType)
            throw std::runtime_error("LinkType mismatch!");

        return *(*linkIter);
    }
    else
    {
        auto linkId = static_cast<int16_t>(_links.size());
        auto linkPtr = std::make_unique<LinkBuilder>(linkType, name, linkId);
        _links.emplace_back(std::move(linkPtr));
        return *(_links[_links.size() - 1]);
    }
}

auto SimulationSetupBuilder::AddParticipant(std::string name) -> ParticipantBuilder&
{
    auto&& participant = std::make_unique<ParticipantBuilder>(this, std::move(name), static_cast<mw::ParticipantId>(_participants.size() + 1));
    _participants.emplace_back(std::move(participant));
    return *_participants[_participants.size() - 1];
}

auto SimulationSetupBuilder::AddSwitch(std::string name) -> SwitchBuilder&
{
    auto&& ethSwitch = std::make_unique<SwitchBuilder>(this, std::move(name));
    _switches.emplace_back(std::move(ethSwitch));
    return *_switches[_switches.size() - 1];
}

auto SimulationSetupBuilder::AddNetworkSimulator(std::string name) -> NetworkSimulatorBuilder&
{
    auto&& networkSimulator = std::make_unique<NetworkSimulatorBuilder>(std::move(name));
    _networkSimulators.emplace_back(std::move(networkSimulator));
    return *_networkSimulators[_networkSimulators.size() - 1];
}

auto SimulationSetupBuilder::ConfigureTimeSync() -> TimeSyncBuilder&
{
    _timeSync = std::make_unique<TimeSyncBuilder>();
    return *_timeSync;
}

auto SimulationSetupBuilder::GetFreeEndpointId() -> mw::EndpointId
{
    return _freeEndpointId++;
}


} // namespace cfg
} // namespace ib
