// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class ReplayBuilder 
{
public:
    IntegrationBusAPI ReplayBuilder(std::string traceSourceName);
    IntegrationBusAPI ~ReplayBuilder();

    IntegrationBusAPI auto operator->() -> ReplayBuilder*;
    IntegrationBusAPI auto Build() -> Replay;

    IntegrationBusAPI auto WithDirection( Replay::Direction dir) -> ReplayBuilder&;

private:
    Replay _replay;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace deprecated
} // namespace cfg
} // namespace ib