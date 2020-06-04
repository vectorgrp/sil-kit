// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ExtensionConfigBuilder.hpp"

namespace ib {
namespace cfg {

auto ExtensionConfigBuilder::AddSearchPath(std::string searchPath) -> ExtensionConfigBuilder&
{
    _config.searchPathHints.emplace_back(std::move(searchPath));
    return *this;
}

auto ExtensionConfigBuilder::Build() -> ExtensionConfig
{
    ExtensionConfig defaultConfig;
    std::swap(_config, defaultConfig);
    return defaultConfig;
}

}
}
