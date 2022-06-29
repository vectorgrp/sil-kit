// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "CreateParticipant.hpp"

namespace ib {
auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                       const std::string& participantName, const uint32_t domainId)
    -> std::unique_ptr<mw::IParticipant>
{
    auto participant = mw::CreateParticipantImpl(std::move(participantConfig), participantName);
    participant->JoinIbDomain(domainId);
    return participant;
}

auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                       std::string participantName)
    -> std::unique_ptr<mw::IParticipant>
{
    return CreateParticipant(participantConfig, participantName, 42);
}

IntegrationBusAPI auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                         std::string participantName, std::string registryUri)
    -> std::unique_ptr<mw::IParticipant>
{
    auto participant = mw::CreateParticipantImpl(std::move(participantConfig), participantName);
    participant->JoinIbDomain(std::move(registryUri));
    return participant;
}

}//namespace ib

