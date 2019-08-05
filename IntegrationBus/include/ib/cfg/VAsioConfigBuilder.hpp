// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"
#include "VAsioRegistryBuilder.hpp"

namespace ib {
namespace cfg {
namespace VAsio {

class ConfigBuilder
{
public:
    IntegrationBusAPI ConfigBuilder();

    IntegrationBusAPI auto ConfigureRegistry() -> RegistryBuilder&;
    IntegrationBusAPI auto Build() -> Config;

private:
    RegistryBuilder _registry;
    Config _config;
};

} // namespace VAsio
} // namespace cfg
} // namespace ib
