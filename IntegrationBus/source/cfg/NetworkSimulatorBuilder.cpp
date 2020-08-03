// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/NetworkSimulatorBuilder.hpp"

namespace ib {
namespace cfg {

NetworkSimulatorBuilder::NetworkSimulatorBuilder(std::string name)
{
    _config.name = std::move(name);
}

auto NetworkSimulatorBuilder::WithLinks(std::initializer_list<std::string> links) -> NetworkSimulatorBuilder&
{
    _config.simulatedLinks.insert(
        _config.simulatedLinks.end(),
        links.begin(), links.end()
    );

    return *this;
}

auto NetworkSimulatorBuilder::WithSwitches(std::initializer_list<std::string> switches) -> NetworkSimulatorBuilder&
{
    _config.simulatedSwitches.insert(
        _config.simulatedSwitches.end(),
        switches.begin(), switches.end()
    );
    return *this;
}

auto NetworkSimulatorBuilder::Build() -> NetworkSimulator
{
    auto builtConfig = _config;
    _config = decltype(_config){};
    return builtConfig;
}

auto NetworkSimulatorBuilder::WithTraceSink(std::string sinkName) -> NetworkSimulatorBuilder&
{
    _config.useTraceSinks.emplace_back(std::move(sinkName));

    return *this;
}

} // namespace cfg
} // namespace ib
