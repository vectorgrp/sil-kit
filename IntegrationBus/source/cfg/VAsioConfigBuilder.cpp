// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConfigBuilder.hpp"

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
    return std::move(_config);
}

} // namespace VAsio
} // namespace cfg
} // namespace ib
