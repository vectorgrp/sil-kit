// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "CreateParticipant.hpp"
#include "Participant.hpp"

namespace ib {
namespace mw {

auto CreateVAsioParticipantImpl(cfg::ParticipantConfiguration participantConfig, const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>
{
    return std::make_unique<Participant<VAsioConnection>>(std::move(participantConfig), participantName);
}

auto CreateParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                           const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>
{
    auto&& cfg = ValidateAndSanitizeConfig(participantConfig, participantName);

    return CreateVAsioParticipantImpl(std::move(cfg), participantName);
}

auto ValidateAndSanitizeConfig(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> cfg::ParticipantConfiguration
{
    if (participantName.empty())
    {
        throw ib::ConfigurationError("An empty participant name is not allowed");
    }

    // try to cast to ParticipantConfiguration to check if the shared pointer is valid
    auto cfg = std::dynamic_pointer_cast<cfg::ParticipantConfiguration>(participantConfig);
    if (cfg == nullptr)
    {
        return {};
    }
    return *cfg;
}

} // namespace mw
} // namespace ib
