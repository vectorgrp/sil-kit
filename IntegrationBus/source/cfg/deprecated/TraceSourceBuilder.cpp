// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TraceSourceBuilder.hpp"

#include <algorithm>

namespace ib {
namespace cfg {
inline namespace deprecated {

TraceSourceBuilder::TraceSourceBuilder(std::string name)
{
	_traceSource.name = std::move(name);
}

TraceSourceBuilder::~TraceSourceBuilder()
{
}

auto TraceSourceBuilder::operator->() -> TraceSourceBuilder*
{
	return this;
}

auto TraceSourceBuilder::Build() -> TraceSource
{
	TraceSource newConfig{};
	std::swap(_traceSource, newConfig);
	return newConfig;
}

auto TraceSourceBuilder::WithType(TraceSource::Type type) -> TraceSourceBuilder&
{
	_traceSource.type = type;
	return *this;
}

auto TraceSourceBuilder::WithInputPath(std::string inputPath) -> TraceSourceBuilder&
{
	_traceSource.inputPath = std::move(inputPath);
	return *this;
}


} // inline namespace deprecated
} // namespace cfg
} // namespace ib

