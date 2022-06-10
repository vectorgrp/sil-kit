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
    participant->joinIbDomain(domainId);
    return participant;
}
}

