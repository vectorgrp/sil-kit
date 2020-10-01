// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TraceSourceBuilder.hpp"


namespace ib {
namespace cfg {

TraceSourceBuilder::TraceSourceBuilder(std::string name)
{
	_traceSource.name = std::move(name);
}

TraceSourceBuilder::~TraceSourceBuilder() = default;

auto TraceSourceBuilder::operator->() -> TraceSourceBuilder*
{
	return this;
}

auto TraceSourceBuilder::Build() -> TraceSource
{
	TraceSource copy = _traceSource;
	_traceSource = TraceSource{};
	return copy;
}

auto TraceSourceBuilder::WithType(TraceSource::Type type) -> TraceSourceBuilder&
{
	_traceSource.type = type;
	return *this;
}

auto TraceSourceBuilder::WithInputPath(std::string outputPath) -> TraceSourceBuilder&
{
	_traceSource.inputPath = std::move(outputPath);
	return *this;
}

auto TraceSourceBuilder::Enabled(bool isEnabled) -> TraceSourceBuilder&
{
	_traceSource.enabled = isEnabled;
	return *this;
}

} // namespace cfg
} // namespace ib

