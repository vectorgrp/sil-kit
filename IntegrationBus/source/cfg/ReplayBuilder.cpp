// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ReplayBuilder.hpp"


namespace ib {
namespace cfg {

ReplayBuilder::ReplayBuilder() = default;
ReplayBuilder::~ReplayBuilder() = default;

auto ReplayBuilder::UseTraceSource(std::string traceSourceName) -> ReplayBuilder&
{
	_replay.useTraceSource = std::move(traceSourceName);
	return *this;
}


auto ReplayBuilder::operator->() -> ReplayBuilder*
{
	return this;
}

auto ReplayBuilder::Build() -> Replay
{
	Replay cfg = _replay;
	_replay = Replay{};
	return cfg;
}

} // namespace cfg
} // namespace ib

