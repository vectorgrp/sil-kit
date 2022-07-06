// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/extensions/IReplay.hpp"

namespace SilKit {
namespace tracing {
class PcapReplay
    : public IReplayDataProvider
{
public:
    // TODO adapt once tracing is reinstated 
    auto OpenFile(/*const Config::Config& config,*/
        const std::string& filePath,
        SilKit::Core::Logging::ILogger* logger)
        -> std::shared_ptr<IReplayFile> override;
};

} // namespace tracing
} // namespace SilKit
