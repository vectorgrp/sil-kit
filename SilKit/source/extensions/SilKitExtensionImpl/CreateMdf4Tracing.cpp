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
