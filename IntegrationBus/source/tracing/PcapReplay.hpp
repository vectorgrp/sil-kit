// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/extensions/IReplay.hpp"

namespace ib {
namespace tracing {
class PcapReplay
    : public extensions::IReplayDataProvider
{
public:
    auto OpenFile(const cfg::Config& config,
        const std::string& filePath,
        ib::mw::logging::ILogger* ibLogger)
        -> std::shared_ptr<extensions::IReplayFile> override;
};

} // namespace tracing
} // namespace ib
