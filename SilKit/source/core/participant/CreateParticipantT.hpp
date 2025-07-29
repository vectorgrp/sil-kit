// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ValidateAndSanitizeConfig.hpp"
#include "Participant.hpp"

namespace SilKit {
namespace Core {

template <typename ConnectionT, typename... Args>
auto CreateParticipantT(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                        const std::string& participantName, const std::string& registryUri,
                        Args&&... args) -> std::unique_ptr<Participant<ConnectionT>>;

// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename ConnectionT, typename... Args>
auto CreateParticipantT(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                        const std::string& participantName, const std::string& registryUri,
                        Args&&... args) -> std::unique_ptr<Participant<ConnectionT>>
{
    auto&& result = ValidateAndSanitizeConfig(std::move(participantConfig), participantName, registryUri);
    auto&& participant = std::make_unique<Participant<ConnectionT>>(std::move(result.participantConfiguration),
                                                                    std::forward<Args>(args)...);

    auto* logger = participant->GetLogger();
    for (const auto& logMessage : result.logMessages)
    {
        logger->Log(logMessage.first, logMessage.second);
    }

    return std::move(participant);
}

} // namespace Core
} // namespace SilKit