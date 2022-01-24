// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class ExtensionConfigBuilder
{
public:
    IntegrationBusAPI ExtensionConfigBuilder() = default;

    IntegrationBusAPI auto AddSearchPath(std::string searchPath) -> ExtensionConfigBuilder&;

    IntegrationBusAPI auto Build() -> ExtensionConfig;

private:
    ExtensionConfig _config;
};
  
} // namespace deprecated
} // namespace cfg
} // namespace ib
