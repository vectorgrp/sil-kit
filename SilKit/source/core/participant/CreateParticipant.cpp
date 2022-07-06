// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "CreateParticipant.hpp"
#include "Participant.hpp"

namespace SilKit {
namespace Core {

auto CreateVAsioParticipantImpl(Config::ParticipantConfiguration participantConfig, const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>
{
    return std::make_unique<Participant<VAsioConnection>>(std::move(participantConfig), participantName);
}

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>
{
    auto&& cfg = ValidateAndSanitizeConfig(participantConfig, participantName);

    return CreateVAsioParticipantImpl(std::move(cfg), participantName);
}

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> Config::ParticipantConfiguration
{
    if (participantName.empty())
    {
        throw SilKit::ConfigurationError("An empty participant name is not allowed");
    }

    // try to cast to ParticipantConfiguration to check if the shared pointer is valid
    auto cfg = std::dynamic_pointer_cast<Config::ParticipantConfiguration>(participantConfig);
    if (cfg == nullptr)
    {
        return {};
    }
    return *cfg;
}

} // namespace Core
} // namespace SilKit
