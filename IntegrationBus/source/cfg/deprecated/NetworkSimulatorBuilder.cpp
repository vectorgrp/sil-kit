// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/NetworkSimulatorBuilder.hpp"

#include <algorithm>

namespace ib {
namespace cfg {
inline namespace deprecated {

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
    NetworkSimulator newConfig;
    std::swap(_config, newConfig);
    return newConfig;
}

auto NetworkSimulatorBuilder::WithTraceSink(std::string sinkName) -> NetworkSimulatorBuilder&
{
    _config.useTraceSinks.emplace_back(std::move(sinkName));

    return *this;
}

} // inline namespace deprecated
} // namespace cfg
} // namespace ib
