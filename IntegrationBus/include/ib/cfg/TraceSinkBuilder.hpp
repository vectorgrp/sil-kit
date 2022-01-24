// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class TraceSinkBuilder 
{
public:
    IntegrationBusAPI TraceSinkBuilder(std::string name);
    IntegrationBusAPI ~TraceSinkBuilder();

    IntegrationBusAPI auto operator->() -> TraceSinkBuilder*;
    IntegrationBusAPI auto Build() -> TraceSink;

    IntegrationBusAPI auto WithType(TraceSink::Type type) -> TraceSinkBuilder&;
    IntegrationBusAPI auto WithOutputPath(std::string outputPath) -> TraceSinkBuilder&;
    IntegrationBusAPI auto Enabled(bool isEnabled) -> TraceSinkBuilder&;

private:
    TraceSink _traceSink{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace deprecated
} // namespace cfg
} // namespace ib
