// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioRegistryBuilder.hpp"

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace cfg {
namespace VAsio {

RegistryBuilder::RegistryBuilder()
{
}

auto RegistryBuilder::WithHostname(std::string hostname) -> RegistryBuilder&
{
    _config.hostname = hostname;
    return *this;
}

auto RegistryBuilder::WithPort(uint16_t port) -> RegistryBuilder&
{
    _config.port = port;
    return *this;
}

auto RegistryBuilder::Build() -> RegistryConfig
{
    return std::move(_config);
}

} // namespace VAsio
} // namespace cfg
} // namespace ib
