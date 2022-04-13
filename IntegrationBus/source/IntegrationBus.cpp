// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "CreateParticipant.hpp"

namespace ib {
auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                 const std::string& participantName, const uint32_t domainId, bool isSynchronized)
    -> std::unique_ptr<mw::IParticipant>
{
    //Validate(config);
    auto participant = mw::CreateParticipantImpl(std::move(participantConfig), participantName, isSynchronized);
    participant->joinIbDomain(domainId);
    return participant;
}
}

