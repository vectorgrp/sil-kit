/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

bool IsValidReplayConfig(const Config::Replay &config);

} // namespace Tracing
} // namespace SilKit
