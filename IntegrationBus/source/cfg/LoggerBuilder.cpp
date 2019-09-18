// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LoggerBuilder.hpp"

#include "ParticipantBuilder.hpp"

namespace ib {
namespace cfg {

LoggerBuilder::LoggerBuilder(ParticipantBuilder *participant)
    : ParentBuilder<ParticipantBuilder>{participant}
{
}

LoggerBuilder::~LoggerBuilder() = default;

auto LoggerBuilder::Build() -> Logger
{
    for (auto&& builder : _sinks)
    {
        _logger.sinks.emplace_back(builder.Build());
    }

    return std::move(_logger);
}

auto LoggerBuilder::AddSink(Sink::Type type) -> SinkBuilder&
{
    _sinks.emplace_back(this, type);
    return _sinks[_sinks.size() - 1];
}

auto LoggerBuilder::EnableLogFromRemotes() -> LoggerBuilder&
{
    _logger.logFromRemotes = true;
    return *this;
}

auto LoggerBuilder::WithFlushLevel(mw::logging::Level level) -> LoggerBuilder&
{
    _logger.flush_level = level;
    return *this;
}

auto LoggerBuilder::operator->() -> LoggerBuilder*
{
    return this;
}

} // namespace cfg
} // namespace ib
