// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {

class TraceSinkBuilder 
{
public:
    IntegrationBusAPI TraceSinkBuilder(std::string name);
    IntegrationBusAPI ~TraceSinkBuilder();

    IntegrationBusAPI auto operator->() -> TraceSinkBuilder*;
    IntegrationBusAPI auto Build() -> TraceSink;

    auto WithType(TraceSink::Type type) -> TraceSinkBuilder&;
    auto WithOutputPath(std::string) -> TraceSinkBuilder&;
    auto Enabled(bool) -> TraceSinkBuilder&;

private:
    TraceSink _traceSink{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib
