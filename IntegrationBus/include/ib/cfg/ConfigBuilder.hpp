// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/IbMacros.hpp"

#include "Config.hpp"
#include "ParticipantBuilder.hpp"
#include "SwitchBuilder.hpp"
#include "NetworkSimulatorBuilder.hpp"
#include "LinkBuilder.hpp"
#include "TimeSyncBuilder.hpp"
#include "FastRtpsConfigBuilder.hpp"
#include "VAsioConfigBuilder.hpp"
#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {

class ConfigBuilder
{
public:
    IntegrationBusAPI ConfigBuilder(std::string name);

    IntegrationBusAPI auto Build() -> Config;

    IntegrationBusAPI auto SimulationSetup() -> SimulationSetupBuilder&;

    IntegrationBusAPI auto ConfigureFastRtps() -> FastRtps::ConfigBuilder&;
    IntegrationBusAPI auto ConfigureVAsio() -> VAsio::ConfigBuilder&;

    IntegrationBusAPI auto WithActiveMiddleware(Middleware middleware) -> ConfigBuilder&;

private:
    Config _config;
    SimulationSetupBuilder _simulationSetup;

    FastRtps::ConfigBuilder _fastRtpsConfig;
    VAsio::ConfigBuilder _vasioConfig;
    MiddlewareConfig _middlewareConfig;
};

} // namespace cfg
} // namespace ib

