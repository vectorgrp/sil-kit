// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {
namespace VAsio {

class RegistryBuilder
{
public:
    IntegrationBusAPI RegistryBuilder();

    IntegrationBusAPI auto WithHostname(std::string hostname) -> RegistryBuilder&;
    IntegrationBusAPI auto WithPort(uint16_t port) -> RegistryBuilder&;

    IntegrationBusAPI auto Build() -> RegistryConfig;

private:
    RegistryConfig _config;
};

} // namespace VAsio
} // namespace cfg
} // namespace ib
