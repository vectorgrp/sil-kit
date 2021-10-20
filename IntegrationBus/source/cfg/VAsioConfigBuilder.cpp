// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConfigBuilder.hpp"

#include <algorithm>

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace cfg {
namespace VAsio {

ConfigBuilder::ConfigBuilder()
{
}

auto ConfigBuilder::ConfigureRegistry() -> RegistryBuilder&
{
    return _registry;
}

auto ConfigBuilder::Build() -> Config
{
    _config.registry = _registry.Build();
    Config newConfig;
    std::swap(_config, newConfig);
    return newConfig;
}
auto ConfigBuilder::WithTcpNoDelay(bool isEnabled) -> ConfigBuilder&
{
    _config.tcpNoDelay = isEnabled;
    return *this;
}

auto ConfigBuilder::WithTcpQuickAck(bool isEnabled) -> ConfigBuilder&
{
    _config.tcpQuickAck = isEnabled;
    return *this;
}
auto ConfigBuilder::WithTcpSendBufferSize(size_t bufferSize) -> ConfigBuilder&
{
    _config.tcpSendBufferSize = bufferSize;
    return *this;
}
auto ConfigBuilder::WithTcpReceiveBufferSize(size_t bufferSize) -> ConfigBuilder&
{
    _config.tcpReceiveBufferSize = bufferSize;
    return *this;
}

} // namespace VAsio
} // namespace cfg
} // namespace ib
