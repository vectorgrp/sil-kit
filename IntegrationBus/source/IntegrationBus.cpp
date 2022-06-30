// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include <sstream>

#include "Validation.hpp"

#include "CreateParticipant.hpp"

namespace {

auto configToUri(ib::cfg::IParticipantConfiguration* userConfig, uint32_t domainId)
{
    auto* cfg = dynamic_cast<ib::cfg::ParticipantConfiguration*>(userConfig);
    std::stringstream uriStr;
    uriStr << "vib://"
        << cfg->middleware.registry.hostname
        << ":"  << std::to_string(cfg->middleware.registry.port + domainId)
        ;
    return uriStr.str();
}

}// namespace
namespace ib {

auto CreateParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                       std::string participantName)
    -> std::unique_ptr<mw::IParticipant>
{
    return CreateParticipant(participantConfig, participantName, configToUri(participantConfig.get(), 0));
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

