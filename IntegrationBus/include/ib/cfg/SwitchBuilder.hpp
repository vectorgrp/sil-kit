// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "fwd_decl.hpp"
#include "Config.hpp"
#include "ParentBuilder.hpp"
#include "SwitchPortBuilder.hpp"

namespace ib {
namespace cfg {

class SwitchBuilder : public ParentBuilder<SimulationSetupBuilder>
{
public:
    IntegrationBusAPI SwitchBuilder(SimulationSetupBuilder* parent, std::string name);

    IntegrationBusAPI auto AddPort(std::string name) -> SwitchPortBuilder&;
    IntegrationBusAPI auto operator->() -> SwitchBuilder*;

    IntegrationBusAPI auto Build() -> Switch;

private:
    Switch _config;
    std::vector<SwitchPortBuilder> _ports;
};

} // namespace cfg
} // namespace ib
