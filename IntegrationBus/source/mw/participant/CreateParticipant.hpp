// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>;

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> SilKit::Config::ParticipantConfiguration;
} // namespace Core
} // namespace SilKit

