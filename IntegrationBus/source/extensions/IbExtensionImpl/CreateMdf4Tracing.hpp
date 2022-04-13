// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

#include "ib/mw/logging/ILogger.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib { namespace extensions {

auto CreateMdf4Tracing(cfg::ParticipantConfiguration config,
    ib::mw::logging::ILogger* logger, const std::string& participantName, const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>;

//////////////////////////////////////////////////////////////////////
// MDF4 Replay
//////////////////////////////////////////////////////////////////////

auto CreateMdf4Replay(cfg::ParticipantConfiguration config, ib::mw::logging::ILogger* logger,
                      const std::string& fileName)
    -> std::shared_ptr<IReplayFile>;

}//end namespace extensions
}//end namespace ib
