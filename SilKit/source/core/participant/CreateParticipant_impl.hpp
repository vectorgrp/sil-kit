// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "CreateParticipant.hpp"
#include "Participant.hpp"

namespace SilKit {
namespace Core {

template <typename ConnectionT>
auto CreateParticipantImplInternal(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                   const std::string& participantName) -> std::unique_ptr<Participant<ConnectionT>>
{
    auto&& cfg = ValidateAndSanitizeConfig(std::move(participantConfig));
    auto&& participantNameValidationResult = ValidateAndSanitizeParticipantName(cfg, participantName);

    auto&& participant =
        std::make_unique<Participant<ConnectionT>>(std::move(cfg), participantNameValidationResult.participantName);

    auto* logger = participant->GetLogger();
    for (const auto& logMessage : participantNameValidationResult.logMessages)
    {
        logger->Log(logMessage.first, logMessage.second);
    }

    return std::move(participant);
}

} // namespace Core
} // namespace SilKit