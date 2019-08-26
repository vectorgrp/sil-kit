// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"

#include "Config.hpp"

#include "fwd_decl.hpp"
#include "ParentBuilder.hpp"

namespace ib {
namespace cfg {

class LoggerBuilder : public ParentBuilder<ParticipantBuilder>
{
public:
    IntegrationBusAPI LoggerBuilder(ParticipantBuilder *participant, cfg::Logger::Type type, mw::logging::Level level);

    IntegrationBusAPI auto WithFilename(std::string filename) -> LoggerBuilder&;

    IntegrationBusAPI auto operator->() -> ParticipantBuilder*;

    IntegrationBusAPI auto Build() -> Logger;

private:
    Logger _logger;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace cfg
} // namespace ib
