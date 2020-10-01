// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {

class TraceSourceBuilder 
{
public:
    IntegrationBusAPI TraceSourceBuilder(std::string name);
    IntegrationBusAPI ~TraceSourceBuilder();

    IntegrationBusAPI auto operator->() -> TraceSourceBuilder*;
    IntegrationBusAPI auto Build() -> TraceSource;

    IntegrationBusAPI auto WithType(TraceSource::Type type) -> TraceSourceBuilder&;
    IntegrationBusAPI auto WithInputPath(std::string) -> TraceSourceBuilder&;
    IntegrationBusAPI auto Enabled(bool) -> TraceSourceBuilder&;

private:
    TraceSource _traceSource{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib
