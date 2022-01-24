// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"
#include "VAsioRegistryBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {
namespace VAsio {

class ConfigBuilder
{
public:
    IntegrationBusAPI ConfigBuilder();

    IntegrationBusAPI auto ConfigureRegistry() -> RegistryBuilder&;
    IntegrationBusAPI auto Build() -> Config;
    IntegrationBusAPI auto WithTcpNoDelay(bool isEnabled) -> ConfigBuilder&;
    IntegrationBusAPI auto WithTcpQuickAck(bool isEnabled) -> ConfigBuilder&;
    IntegrationBusAPI auto WithTcpSendBufferSize(size_t bufferSize) -> ConfigBuilder&;
    IntegrationBusAPI auto WithTcpReceiveBufferSize(size_t bufferSize) -> ConfigBuilder&;

private:
    RegistryBuilder _registry;
    Config _config;
};

} // namespace VAsio
} // namespace deprecated
} // namespace cfg
} // namespace ib
