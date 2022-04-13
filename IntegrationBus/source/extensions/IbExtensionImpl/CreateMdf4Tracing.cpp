// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>

// concrete type needed for interface
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"

#include "IbExtensions.hpp"
#include "CreateMdf4Tracing.hpp"
#include "FactorySingleton.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib { namespace extensions {

auto CreateMdf4Tracing(cfg::ParticipantConfiguration config,
    ib::mw::logging::ILogger* logger,
    const std::string& participantName,
    const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>
{
    // TODO fix factory analogous to CreateIbRegistry once tracing will be reinstated
    auto& factory = FactorySingleton<ITraceMessageSinkFactory>(logger, "vibe-mdf4tracing", config.extensions);
    return factory.Create(/*std::move(config), */logger, participantName, sinkName);
}

auto CreateMdf4Replay(cfg::ParticipantConfiguration config, ib::mw::logging::ILogger* logger,
                      const std::string& fileName)
    -> std::shared_ptr<IReplayFile>
{
    // TODO fix factory analogous to CreateIbRegistry once tracing will be reinstated
    auto& factory = FactorySingleton<IReplayDataProvider>(logger, "vibe-mdf4tracing", config.extensions);
    return factory.OpenFile(/*config, */fileName, logger);
}

}//end namespace extensions
}//end namespace ib
