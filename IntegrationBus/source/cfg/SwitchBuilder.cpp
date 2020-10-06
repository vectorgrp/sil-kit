// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SwitchBuilder.hpp"

#include <algorithm>

#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {

SwitchBuilder::SwitchBuilder(SimulationSetupBuilder* parent, std::string name)
    : ParentBuilder<SimulationSetupBuilder>{parent}
{
    _config.name = std::move(name);
}

auto SwitchBuilder::AddPort(std::string name) -> SwitchPortBuilder&
{
    auto freeEndpointId = Parent()->GetFreeEndpointId();
    _ports.emplace_back(this, std::move(name), freeEndpointId);
    return _ports[_ports.size() - 1];
}

auto SwitchBuilder::operator->() -> SwitchBuilder*
{
    return this;
}

auto SwitchBuilder::Build() -> Switch
{
    for (auto&& builder : _ports)
    {
        _config.ports.emplace_back(builder.Build());
    }
    _ports.clear();
    Switch newConfig{};
    std::swap(_config, newConfig);
    return newConfig;
}


} // namespace cfg
} // namespace ib
