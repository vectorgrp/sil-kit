// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <ostream>

#include "silkit/services/fwd_decl.hpp"
#include "silkit/services/logging/fwd_decl.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Tracing {

// Configure the trace sinks based on the configuration and return a vector of
// the sinks.
auto CreateTraceMessageSinks(Services::Logging::ILogger* logger,
                             const Config::ParticipantConfiguration& participantConfig)
    -> std::vector<std::unique_ptr<ITraceMessageSink>>;

// Configure replay files from the trace source configurations and return a vector of
// the files.
auto CreateReplayFiles(Services::Logging::ILogger* logger, const Config::ParticipantConfiguration& participantConfig)
    -> std::map<std::string, std::shared_ptr<IReplayFile>>;

//! \brief Predicate to check whether any of the participant's controllers
//         has a Replay config.
bool HasReplayConfig(const Config::ParticipantConfiguration& config);

bool IsValidReplayConfig(const Config::Replay& config);

} // namespace Tracing
} // namespace SilKit
