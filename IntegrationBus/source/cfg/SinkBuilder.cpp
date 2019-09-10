// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SinkBuilder.hpp"

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

auto SinkBuilder::WithFilename(std::string filename) -> SinkBuilder&
{
    _sink.filename = filename;
    return *this;
}

auto SinkBuilder::operator->() -> LoggerBuilder*
{
    return Parent();
}

auto SinkBuilder::Build() -> Sink
{
    if (_sink.type == cfg::Sink::Type::File && _sink.filename.empty())
        throw Misconfiguration("A file sink needs a filename to be set!");
    return std::move(_sink);
}

} // namespace cfg
} // namespace ib
