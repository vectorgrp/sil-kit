// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "CreateParticipant.hpp"
#include "CreateParticipant_impl.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName) -> std::unique_ptr<IParticipantInternal>
{
    return CreateParticipantImplInternal<VAsioConnection>(std::move(participantConfig), participantName);
}

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig)
    -> Config::ParticipantConfiguration
{
    // try to cast to ParticipantConfiguration to check if the shared pointer is valid
    auto cfg = std::dynamic_pointer_cast<Config::ParticipantConfiguration>(participantConfig);
    if (cfg == nullptr)
    {
        return {};
    }

    return *cfg;
}

auto ValidateAndSanitizeParticipantName(Config::ParticipantConfiguration participantConfig,
                                        const std::string& participantName) -> ParticipantNameValidationResult
{
    if (participantName.empty())
    {
        throw SilKit::ConfigurationError("An empty participant name is not allowed");
    }

    ParticipantNameValidationResult result{participantName, {}};

    if (!participantConfig.participantName.empty() && participantConfig.participantName != result.participantName)
    {
        result.participantName = participantConfig.participantName;
        result.logMessages.emplace_back(
            Services::Logging::Level::Info,
            fmt::format("The provided participant name '{}' differs from the configured name '{}'. The latter will be "
                        "used.",
                        participantName, participantConfig.participantName));
    }

    return result;
}

} // namespace Core
} // namespace SilKit