// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "ConfigBuilder.hpp"
#include "Config.hpp"

#include <algorithm>

namespace ib {
namespace cfg {


ConfigBuilder::ConfigBuilder(std::string name)
{
    _config.name = std::move(name);
}

auto ConfigBuilder::Build() -> Config
{
    _config.simulationSetup = _simulationSetup.Build();

    if (_fastRtpsConfig)
        _config.middlewareConfig.fastRtps = _fastRtpsConfig->Build();

    // Post-processing steps
    // Note: Some steps (AssignEndpointAddresses, AssignLinkIds) are done by this builder on-the-fly
    UpdateGenericSubscribers(_config);

    return std::move(_config);
}

auto ConfigBuilder::SimulationSetup() -> SimulationSetupBuilder&
{
    return _simulationSetup;
}

auto ConfigBuilder::ConfigureFastRtps() -> FastRtps::ConfigBuilder&
{
    _fastRtpsConfig = std::make_unique<FastRtps::ConfigBuilder>();
    return *_fastRtpsConfig;
}



} // namespace cfg
} // namespace ib
