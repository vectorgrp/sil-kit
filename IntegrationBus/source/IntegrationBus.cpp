// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"
#include "ib/cfg/string_utils.hpp"

#include "CreateComAdapter.hpp"

namespace ib {

namespace { // anonymous namespace
void PatchConfigForVAsio(cfg::Config& config)
{
    if (config.middlewareConfig.activeMiddleware != cfg::Middleware::VAsio)
        return;

    for (auto& participant : config.simulationSetup.participants)
    {
        auto& participantController = participant.participantController;
        if (!participantController)
            continue;

        switch (participantController->syncType)
        {
        case cfg::SyncType::DistributedTimeQuantum: // [[fallthrough]]
        case cfg::SyncType::Unsynchronized:
            break;
        case cfg::SyncType::DiscreteTime:        // [[fallthrough]]
        case cfg::SyncType::DiscreteTimePassive: // [[fallthrough]]
        case cfg::SyncType::DiscreteEvent:       // [[fallthrough]]
        case cfg::SyncType::TimeQuantum:         // [[fallthrough]]
        {
            static bool warnOnce = true;
            if (warnOnce)
            {
                std::cout << "WARNING: Usage of SyncType::" << to_string(participantController->syncType) << " is deprecated when using VAsio middleware\n";
                warnOnce = false;
            }
            std::cout << "INFO: Overriding SyncType for participant \"" << participant.name << "\" to SyncType::DistributedTimeQuantum\n";
            participantController->syncType = cfg::SyncType::DistributedTimeQuantum;
        }
        }
    }
}
} // anonymous namespace

auto CreateComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
{
    Validate(config);
    if (config.middlewareConfig.activeMiddleware == cfg::Middleware::VAsio)
    {
        PatchConfigForVAsio(config);
    }
    auto comAdapter = mw::CreateComAdapterImpl(std::move(config), participantName);
    comAdapter->joinIbDomain(domainId);
    return comAdapter;
}

auto CreateSimulationParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                 const std::string& participantName, const uint32_t domainId, cfg::Config config)
    -> std::unique_ptr<mw::IComAdapter>
{
    //Validate(config);
    auto comAdapter = mw::CreateSimulationParticipantImpl(std::move(participantConfig), participantName, std::move(config));
    comAdapter->joinIbDomain(domainId);
    return comAdapter;
}
}

