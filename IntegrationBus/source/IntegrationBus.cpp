// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"
#include "CreateParticipant.hpp"
#include "ParticipantConfiguration.hpp"


namespace ib {

auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                       const std::string& participantName)
    -> std::unique_ptr<mw::IParticipant>
{
    const auto uri = dynamic_cast<cfg::ParticipantConfiguration&>(*participantConfig).middleware.registryUri;
    return CreateParticipant(participantConfig, participantName, uri);
}

IntegrationBusAPI auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<mw::IParticipant>
{
    auto participant = mw::CreateParticipantImpl(std::move(participantConfig), participantName);
    participant->JoinIbDomain(registryUri);
    return participant;
}

}//namespace ib

