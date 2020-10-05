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
	return std::move(_replay);
}


auto ReplayBuilder::WithDirection( ReplayConfig::Direction dir) -> ReplayBuilder&
{
	ReplayConfig cfg;
	cfg.direction = dir;
	_replay.withReplayConfigs.emplace_back(std::move(cfg));

	return *this;
}
} // namespace cfg
} // namespace ib

