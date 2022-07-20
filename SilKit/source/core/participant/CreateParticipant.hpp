// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName) -> std::unique_ptr<IParticipantInternal>;

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig)
    -> SilKit::Config::ParticipantConfiguration;

struct ParticipantNameValidationResult
{
    std::string participantName;
    std::vector<std::pair<Services::Logging::Level, std::string>> logMessages;
};

auto ValidateAndSanitizeParticipantName(Config::ParticipantConfiguration participantConfig,
                                        const std::string& participantName) -> ParticipantNameValidationResult;

} // namespace Core
} // namespace SilKit