// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Config.hpp"

#include "ib/IbMacros.hpp"

namespace ib {
namespace cfg {

class NetworkSimulatorBuilder
{
public:
    IntegrationBusAPI NetworkSimulatorBuilder(std::string name);

    IntegrationBusAPI auto WithLinks(std::initializer_list<std::string> links) -> NetworkSimulatorBuilder&;
    IntegrationBusAPI auto WithSwitches(std::initializer_list<std::string> switches) -> NetworkSimulatorBuilder&;

    IntegrationBusAPI auto Build() -> NetworkSimulator;

private:
    NetworkSimulator _config;
};

} // namespace cfg
} // namespace ib
