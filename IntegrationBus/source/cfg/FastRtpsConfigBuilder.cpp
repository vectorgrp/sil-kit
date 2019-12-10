// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FastRtpsConfigBuilder.hpp"

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace cfg {
namespace FastRtps {

ConfigBuilder::ConfigBuilder()
{
}

auto ConfigBuilder::WithDiscoveryType(DiscoveryType discoveryType) -> ConfigBuilder&
{
    _config.discoveryType = discoveryType;
    return *this;
}

auto ConfigBuilder::AddUnicastLocator(std::string participantName, std::string ipAddress) -> ConfigBuilder&
{
    _config.unicastLocators[std::move(participantName)] = std::move(ipAddress);
    return *this;
}

auto ConfigBuilder::WithConfigFileName(std::string fileName) -> ConfigBuilder&
{
    _config.configFileName = std::move(fileName);
    return *this;
}

auto ConfigBuilder::WithHistoryDepth(int historyDepth) -> ConfigBuilder&
{
    if (historyDepth <= 0)
        throw Misconfiguration{"FastRTPS HistoryDepth must be above 0"};

    _config.historyDepth = historyDepth;
    return *this;
}

auto ConfigBuilder::Build() -> Config
{
    switch (_config.discoveryType)
    {
    case FastRtps::DiscoveryType::Local:
        if (!_config.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Local"};

        if (!_config.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::Multicast:
        if (!_config.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Multicast"};

        if (!_config.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::Unicast:
        if (_config.unicastLocators.empty())
            throw Misconfiguration{"DiscoveryType Unicast requires UnicastLocators being specified"};

        if (!_config.configFileName.empty())
            throw Misconfiguration{"Using a FastRTPS configuration file requires DiscoverType ConfigFile"};

        break;

    case FastRtps::DiscoveryType::ConfigFile:
        if (!_config.unicastLocators.empty())
            throw Misconfiguration{"UnicastLocators must not be specified when using DiscoveryType Multicast"};

        if (_config.configFileName.empty())
            throw Misconfiguration{"DiscoveryType ConfigFile requires ConfigFileName being specified"};

        break;

    default:
        throw Misconfiguration{"Invalid FastRTPS discovery type: " + to_string(_config.discoveryType)};
    }

    return std::move(_config);
}

} // namespace FastRtps
} // namespace cfg
} // namespace ib
