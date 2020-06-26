// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>

// concrete type needed for interface
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"
#include "ib/extensions/ITraceMessageSink.hpp"

#include "IbExtensions.hpp"
#include "CreateMdf4Tracing.hpp"
#include "CreateInstance.hpp"

namespace ib { namespace extensions {

auto CreateMdf4Tracing(cfg::Config config,
    ib::mw::logging::ILogger* logger,
    const std::string& sinkName)
    -> ExtensionHandle<ITraceMessageSink>
{
    return CreateInstance<Mdf4TraceSinkFactory, ITraceMessageSink>
        ("vibe-mdf4tracing", std::move(config), logger, sinkName);
}

}//end namespace extensions
}//end namespace ib
