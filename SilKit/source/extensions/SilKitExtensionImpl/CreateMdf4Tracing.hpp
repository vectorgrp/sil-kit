// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/services/logging/ILogger.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"

#include "ParticipantConfiguration.hpp"

namespace SilKit {

auto CreateMdf4Tracing(Config::ParticipantConfiguration config, SilKit::Services::Logging::ILogger* logger,
                       const std::string& participantName,
                       const std::string& sinkName) -> std::unique_ptr<ITraceMessageSink>;

//////////////////////////////////////////////////////////////////////
// MDF4 Replay
//////////////////////////////////////////////////////////////////////

auto CreateMdf4Replay(Config::ParticipantConfiguration config, SilKit::Services::Logging::ILogger* logger,
                      const std::string& fileName) -> std::shared_ptr<IReplayFile>;


} //end namespace SilKit
