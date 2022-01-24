// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

class SinkBuilder : public ParentBuilder<LoggerBuilder>
{
public:
    IntegrationBusAPI SinkBuilder(LoggerBuilder *logger, cfg::Sink::Type type);

    IntegrationBusAPI auto WithLogLevel(mw::logging::Level level) -> SinkBuilder&;

    IntegrationBusAPI auto WithLogname(std::string logname) -> SinkBuilder&;

    IntegrationBusAPI auto operator->() -> LoggerBuilder*;

    IntegrationBusAPI auto Build() -> Sink;

private:
    Sink _sink;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace deprecated
} // namespace cfg
} // namespace ib
