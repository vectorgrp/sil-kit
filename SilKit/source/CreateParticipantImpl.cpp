// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateParticipantImpl.hpp"

#include "CreateParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName,
                           const std::string& registryUri) -> std::unique_ptr<IParticipant>
{
    auto participant = Core::CreateParticipantInternal(std::move(participantConfig), participantName, registryUri);
    participant->JoinSilKitSimulation();
    return participant;
}

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName) -> std::unique_ptr<IParticipant>
{
    const auto uri = dynamic_cast<Config::ParticipantConfiguration&>(*participantConfig).middleware.registryUri;
    return CreateParticipantImpl(participantConfig, participantName, uri);
}

auto GetLoggerInternal(SilKit::IParticipant* participant) -> SilKit::Services::Logging::ILoggerInternal*
{
    auto participantImpl = dynamic_cast<Core::IParticipantInternal*>(participant);
    if (participantImpl == nullptr)
    {
        throw SilKit::ConfigurationError("The provided participant is not a valid SilKit::Core::IParticipantInternal");
    }
    return participantImpl->GetLoggerInternal();
}

} //namespace SilKit
