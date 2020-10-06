// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SinkBuilder.hpp"

#include <algorithm>

#include "LoggerBuilder.hpp"

namespace ib {
namespace cfg {

SinkBuilder::SinkBuilder(LoggerBuilder *logger, cfg::Sink::Type type)
    : ParentBuilder<LoggerBuilder>{logger}
{
    _sink.type = type;
}

auto SinkBuilder::WithLogLevel(mw::logging::Level level) -> SinkBuilder&
{
    _sink.level = level;
    return *this;
}

auto SinkBuilder::WithLogname(std::string logname) -> SinkBuilder&
{
    _sink.logname = logname;
    return *this;
}

auto SinkBuilder::operator->() -> LoggerBuilder*
{
    return Parent();
}

auto SinkBuilder::Build() -> Sink
{
    if (_sink.type == cfg::Sink::Type::File && _sink.logname.empty())
        throw Misconfiguration("A file sink needs a logname to be set!");
    Sink newConfig{};
    std::swap(_sink, newConfig);
    return newConfig;
}

} // namespace cfg
} // namespace ib
