// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
    _config.middlewareConfig.fastRtps = _fastRtpsConfig.Build();
    _config.middlewareConfig.vasio = _vasioConfig.Build();

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
    _config.middlewareConfig.activeMiddleware = Middleware::FastRTPS;
    return _fastRtpsConfig;
}

auto ConfigBuilder::ConfigureVAsio() -> VAsio::ConfigBuilder&
{
    _config.middlewareConfig.activeMiddleware = Middleware::VAsio;
    return _vasioConfig;
}

auto ConfigBuilder::WithActiveMiddleware(Middleware middleware) -> ConfigBuilder&
{
    _config.middlewareConfig.activeMiddleware = middleware;
    return *this;
}

} // namespace cfg
} // namespace ib
