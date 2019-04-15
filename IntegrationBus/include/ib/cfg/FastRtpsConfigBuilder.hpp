// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {
namespace FastRtps {

class ConfigBuilder
{
public:
    IntegrationBusAPI ConfigBuilder();

    IntegrationBusAPI auto WithDiscoveryType(DiscoveryType discoveryType) -> ConfigBuilder&;
    IntegrationBusAPI auto AddUnicastLocator(std::string participantName, std::string ipAddress) -> ConfigBuilder&;
    IntegrationBusAPI auto WithConfigFileName(std::string fileName) -> ConfigBuilder&;

    IntegrationBusAPI auto Build() -> Config;

private:
    Config _config;
};

} // namespace FastRtps
} // namespace cfg
} // namespace ib
