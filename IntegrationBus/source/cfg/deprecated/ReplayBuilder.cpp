// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ReplayBuilder.hpp"
#include <algorithm>


namespace ib {
namespace cfg {
inline namespace deprecated {

ReplayBuilder::ReplayBuilder(std::string traceSourceName)
{
	_replay.useTraceSource = std::move(traceSourceName);
}

ReplayBuilder::~ReplayBuilder() = default;

auto ReplayBuilder::operator->() -> ReplayBuilder*
{
	return this;
}

auto ReplayBuilder::Build() -> Replay
{
	Replay newConfig{};
	std::swap(_replay, newConfig);
	return newConfig;
}


auto ReplayBuilder::WithDirection( Replay::Direction dir) -> ReplayBuilder&
{
	_replay.direction = dir;

	return *this;
}
} // inline namespace deprecated
} // namespace cfg
} // namespace ib

