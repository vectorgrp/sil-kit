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
#include "VAsioConfigBuilder.hpp"
#include "SimulationSetupBuilder.hpp"
#include "ExtensionConfigBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class ConfigBuilder
{
public:
    /*! \brief create a named configuration builder.
    *
    * The builder pattern allows creating a valid configuration incrementally
    * in a declarative style.
    * When done adding or declaring components using the sub-builds, a call to
    * ::Build() will create a Config object.
    */
    IntegrationBusAPI ConfigBuilder(std::string name);
    //! \brief Recursively build a configuration object including all previously declared sub-builders.
    IntegrationBusAPI auto Build() -> Config;
    //! \brief Add a simulation setup builder to the current configuration builder.
    IntegrationBusAPI auto SimulationSetup() -> SimulationSetupBuilder&;
    //! \brief Add a builder for configuring the VAsio middleware.
    IntegrationBusAPI auto ConfigureVAsio() -> VAsio::ConfigBuilder&;
    //! \brief Declare the current active middleware to use.
    IntegrationBusAPI auto WithActiveMiddleware(Middleware middleware) -> ConfigBuilder&;
    //! \brief Add builder for configuring the extensions
    IntegrationBusAPI auto ConfigureExtensions() -> ExtensionConfigBuilder&;

private:
    Config _config;
    std::unique_ptr<SimulationSetupBuilder> _simulationSetup;
    std::unique_ptr<VAsio::ConfigBuilder> _vasioConfig;
    ExtensionConfigBuilder _extensionConfig;
};

} // namespace deprecated
} // namespace cfg
} // namespace ib

