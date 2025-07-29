// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>

// concrete type needed for interface
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"

#include "ITraceMessageSink.hpp"
#include "IReplay.hpp"

#include "SilKitExtensions.hpp"
#include "CreateMdf4Tracing.hpp"
#include "SilKitExtensionLoader.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {

auto CreateMdf4Tracing(Config::ParticipantConfiguration config, SilKit::Services::Logging::ILogger* logger,
                       const std::string& participantName,
                       const std::string& sinkName) -> std::unique_ptr<ITraceMessageSink>
{
    auto& factory = SilKitExtensionLoader<ITraceMessageSinkFactory>(logger, "SilKitExtension_Mdf", config.extensions);
    return factory.Create(std::move(config), logger, participantName, sinkName);
}

auto CreateMdf4Replay(Config::ParticipantConfiguration config, SilKit::Services::Logging::ILogger* logger,
                      const std::string& fileName) -> std::shared_ptr<IReplayFile>
{
    auto& factory = SilKitExtensionLoader<IReplayDataProvider>(logger, "SilKitExtension_Mdf", config.extensions);
    return factory.OpenFile(config, fileName, logger);
}


} //end namespace SilKit
