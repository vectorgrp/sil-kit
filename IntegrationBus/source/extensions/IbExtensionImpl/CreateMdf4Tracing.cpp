// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>

// concrete type needed for interface
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/extensions/IReplay.hpp"

#include "IbExtensions.hpp"
#include "CreateMdf4Tracing.hpp"
#include "CreateInstance.hpp"

namespace ib { namespace extensions {

auto CreateMdf4Tracing(cfg::Config config,
    ib::mw::logging::ILogger* logger,
    const std::string& participantName,
    const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>
{
    auto& factory = CreateInstance<ITraceMessageSinkFactory>("vibe-mdf4tracing", config);
    return factory.Create(std::move(config), logger, participantName, sinkName);
}

auto CreateMdf4Replay(cfg::Config config, ib::mw::logging::ILogger* logger, const std::string& fileName)
    -> std::shared_ptr<IReplayFile>
{
    auto& factory = CreateInstance<IReplayDataProvider>("vibe-mdf4replay", config);
    return factory.OpenFile(fileName, logger);
}

}//end namespace extensions
}//end namespace ib
