// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"

namespace ib {
namespace cfg {

class ReplayBuilder 
{
public:
    IntegrationBusAPI ReplayBuilder();
    IntegrationBusAPI ~ReplayBuilder();

    IntegrationBusAPI auto operator->() -> ReplayBuilder*;
    IntegrationBusAPI auto Build() -> Replay;

    IntegrationBusAPI auto UseTraceSource(std::string traceSourceName) -> ReplayBuilder&;

private:
    Replay _replay{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib