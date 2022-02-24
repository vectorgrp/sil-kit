// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "CreateComAdapter.hpp"

namespace ib {
auto CreateSimulationParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                 const std::string& participantName, const uint32_t domainId, bool isSynchronized)
    -> std::unique_ptr<mw::IComAdapter>
{
    //Validate(config);
    auto comAdapter = mw::CreateSimulationParticipantImpl(std::move(participantConfig), participantName, isSynchronized);
    comAdapter->joinIbDomain(domainId);
    return comAdapter;
}
}

