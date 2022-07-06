// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <ostream>

#include "silkit/services/fwd_decl.hpp"

#include "silkit/cfg/fwd_decl.hpp"
#include "silkit/services/logging/fwd_decl.hpp"

#include "silkit/extensions/ITraceMessageSink.hpp"
#include "silkit/extensions/IReplay.hpp"

namespace SilKit {
namespace tracing {

// Configure the trace sinks based on the configuration and return a vector of
// the sinks.

auto CreateTraceMessageSinks(
    Services::Logging::ILogger* logger,
    const Config::Config& config,
    const Config::Participant& participantConfig
    ) -> std::vector<std::unique_ptr<ITraceMessageSink>>;

// Configure replay files from the trace source configurations and return a vector of
// the files.
auto CreateReplayFiles(
    Services::Logging::ILogger* logger,
    const Config::Config& config,
    const Config::Participant& participantConfig
    ) -> std::map<std::string, std::shared_ptr<IReplayFile>>;

//! \brief Predicate to check whether any of the participant's controllers
//         has a Replay config.

bool HasReplayConfig(const Config::Participant& config);
} //end namespace tracing
} //end namespace SilKit
