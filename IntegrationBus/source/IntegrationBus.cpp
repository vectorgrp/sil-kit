// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include <sstream>

#include "Validation.hpp"

#include "CreateParticipant.hpp"

namespace {

auto configToUri(ib::cfg::IParticipantConfiguration* userConfig)
{
    auto* cfg = dynamic_cast<ib::cfg::ParticipantConfiguration*>(userConfig);
    std::stringstream uriStr;
    uriStr << "vib://"
        << cfg->middleware.registry.hostname
        << ":"  << std::to_string(cfg->middleware.registry.port)
        ;
    return uriStr.str();
}

}// namespace

namespace ib {

auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                       const std::string& participantName)
    -> std::unique_ptr<mw::IParticipant>
{
    const auto uri = configToUri(participantConfig.get());
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

