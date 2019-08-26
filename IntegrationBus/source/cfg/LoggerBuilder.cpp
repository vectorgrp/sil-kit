// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LoggerBuilder.hpp"

#include "ParticipantBuilder.hpp"
#include "SimulationSetupBuilder.hpp"

namespace ib {
namespace cfg {

LoggerBuilder::LoggerBuilder(ParticipantBuilder *participant, cfg::Logger::Type type, mw::logging::Level level)
    : ParentBuilder<ParticipantBuilder>{participant}
{
    _logger.type = type;
    _logger.level = level;
}

auto LoggerBuilder::WithFilename(std::string filename) -> LoggerBuilder&
{
    _logger.filename = filename;
    return *this;
}

auto LoggerBuilder::operator->() -> ParticipantBuilder*
{
    return Parent();
}

auto LoggerBuilder::Build() -> Logger
{
    return std::move(_logger);
}

} // namespace cfg
} // namespace ib
