// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Core {

struct ValidateAndSanitizeConfigResult
{
    SilKit::Config::ParticipantConfiguration participantConfiguration;
    std::vector<std::pair<Services::Logging::Level, std::string>> logMessages;
};

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName,
                               const std::string& registryUri) -> ValidateAndSanitizeConfigResult;

} // namespace Core
} // namespace SilKit
