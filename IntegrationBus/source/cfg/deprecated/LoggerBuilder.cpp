// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LoggerBuilder.hpp"

#include <algorithm>

#include "ParticipantBuilder.hpp"

namespace ib {
namespace cfg {
inline namespace deprecated {

LoggerBuilder::~LoggerBuilder() = default;

auto LoggerBuilder::Build() -> Logger
{
    for (auto&& builder : _sinks)
    {
        _logger.sinks.emplace_back(builder.Build());
    }

    Logger newConfig{};
    std::swap(_logger, newConfig);
    return newConfig;
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

} // inline namespace deprecated
} // namespace cfg
} // namespace ib
