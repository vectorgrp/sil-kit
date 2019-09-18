// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"

#include "SinkBuilder.hpp"

namespace ib {
namespace cfg {

class LoggerBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI LoggerBuilder(ParticipantBuilder *participant);
    IntegrationBusAPI ~LoggerBuilder();

    IntegrationBusAPI auto AddSink(Sink::Type type) -> SinkBuilder&;

    IntegrationBusAPI auto EnableLogFromRemotes() -> LoggerBuilder&;
    IntegrationBusAPI auto WithFlushLevel(mw::logging::Level level) -> LoggerBuilder&;

    IntegrationBusAPI auto operator->() -> LoggerBuilder*;

    IntegrationBusAPI auto Build() -> Logger;

private:
    Logger _logger;

    std::vector<SinkBuilder> _sinks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib
