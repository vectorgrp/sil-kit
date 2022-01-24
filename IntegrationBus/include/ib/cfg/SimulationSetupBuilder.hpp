// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/IbMacros.hpp"

#include "Config.hpp"
#include "LinkBuilder.hpp"
#include "NetworkSimulatorBuilder.hpp"
#include "ParticipantBuilder.hpp"
#include "SwitchBuilder.hpp"
#include "TimeSyncBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class SimulationSetupBuilder
{
public:
    IntegrationBusAPI SimulationSetupBuilder() = default;

    IntegrationBusAPI auto Build() -> SimulationSetup;

    IntegrationBusAPI auto AddParticipant(std::string name) -> ParticipantBuilder&;
    IntegrationBusAPI auto AddSwitch(std::string name) -> SwitchBuilder&;

    IntegrationBusAPI auto AddOrGetLink(Link::Type linkType, const std::string& name) -> LinkBuilder&;

    IntegrationBusAPI auto ConfigureTimeSync() -> TimeSyncBuilder&;

public:
    // IB Internal 
    auto GetFreeEndpointId() -> mw::EndpointId;

private:
    SimulationSetup _config;

    std::vector<std::unique_ptr<ParticipantBuilder>> _participants;
    std::vector<std::unique_ptr<SwitchBuilder>> _switches;

    std::vector<std::unique_ptr<LinkBuilder>> _links;

    std::unique_ptr<TimeSyncBuilder> _timeSync;

    mw::EndpointId _freeEndpointId = 1;
};

} // namespace deprecated
} // namespace cfg
} // namespace ib
