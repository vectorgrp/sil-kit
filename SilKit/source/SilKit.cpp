// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SilKit.hpp"

#include "Validation.hpp"
#include "CreateParticipant.hpp"
#include "ParticipantConfiguration.hpp"


namespace SilKit {

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName)
    -> std::unique_ptr<IParticipant>
{
    const auto uri = dynamic_cast<Config::ParticipantConfiguration&>(*participantConfig).middleware.registryUri;
    return CreateParticipant(participantConfig, participantName, uri);
}

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<IParticipant>
{
    auto participant = Core::CreateParticipantImpl(std::move(participantConfig), participantName);
    participant->JoinSilKitSimulation(registryUri);
    return participant;
}

}//namespace SilKit

