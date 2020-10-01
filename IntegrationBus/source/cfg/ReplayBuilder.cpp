// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ReplayBuilder.hpp"


namespace ib {
namespace cfg {

ReplayBuilder::ReplayBuilder(std::string name)
{
	_replay.useTraceSource = std::move(name);
}

ReplayBuilder::~ReplayBuilder() = default;

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

