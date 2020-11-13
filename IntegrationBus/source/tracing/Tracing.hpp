// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <ostream>

#include "ib/sim/fwd_decl.hpp"
#include "ib/mw/EndpointAddress.hpp"

#include "ib/cfg/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"

#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/extensions/IReplay.hpp"

namespace ib {
namespace tracing {

// Configure the trace sinks based on the configuration and return a vector of
// the sinks.

auto CreateTraceMessageSinks(
    mw::logging::ILogger* logger,
    const cfg::Config& config,
    const cfg::Participant& participantConfig
    ) -> std::vector<std::unique_ptr<extensions::ITraceMessageSink>>;

// Configure replay files from the trace source configurations and return a vector of
// the files.
auto CreateReplayFiles(
    mw::logging::ILogger* logger,
    const cfg::Config& config,
    const cfg::Participant& participantConfig
    ) -> std::map<std::string, std::shared_ptr<extensions::IReplayFile>>;

//! \brief Predicate to check whether any of the participant's controllers
//         has a Replay config.

bool HasReplayConfig(const cfg::Participant& config);
} //end namespace tracing
} //end namespace ib
