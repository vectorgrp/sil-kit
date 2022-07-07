// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>

// concrete type needed for interface
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"

#include "SilKitExtensions.hpp"
#include "CreateMdf4Tracing.hpp"
#include "FactorySingleton.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit { 

auto CreateMdf4Tracing(Config::ParticipantConfiguration config,
    SilKit::Services::Logging::ILogger* logger,
    const std::string& participantName,
    const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>
{
    // TODO fix factory analogous to CreateSilKitRegistry once tracing will be reinstated
    auto& factory = FactorySingleton<ITraceMessageSinkFactory>(logger, "vibe-mdf4tracing", config.extensions);
    return factory.Create(/*std::move(config), */logger, participantName, sinkName);
}

auto CreateMdf4Replay(Config::ParticipantConfiguration config, SilKit::Services::Logging::ILogger* logger,
                      const std::string& fileName)
    -> std::shared_ptr<IReplayFile>
{
    // TODO fix factory analogous to CreateSilKitRegistry once tracing will be reinstated
    auto& factory = FactorySingleton<IReplayDataProvider>(logger, "vibe-mdf4tracing", config.extensions);
    return factory.OpenFile(/*config, */fileName, logger);
}


}//end namespace SilKit
