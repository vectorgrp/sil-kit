// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/services/logging/ILogger.hpp"
#include "IReplay.hpp"

namespace SilKit {
namespace Tracing {

class PcapReplay : public IReplayDataProvider
{
public:
    auto OpenFile(const SilKit::Config::ParticipantConfiguration&, const std::string& filePath,
                  SilKit::Services::Logging::ILogger* logger) -> std::shared_ptr<IReplayFile> override;
};

} // namespace Tracing
} // namespace SilKit
