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
        switch (participant.syncType)
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
                std::cout << "WARNING: Usage of SyncType::" << to_string(participant.syncType) << " is deprecated when using VAsio middleware\n";
                warnOnce = false;
            }
            std::cout << "INFO: overriding SyncType for participant \"" << participant.name << "\" to SyncType::DistributedTimeQuantum\n";
            participant.syncType = cfg::SyncType::DistributedTimeQuantum;
        }
        }
    }
}
} // anonymous namespace

auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
{
    if (config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::VAsio)
    {
        std::cout << "Creating FastRTPS ComAdapter but VAsio ComAdapter was configured in IbConfig!" << std::endl;
    }
    config.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;
    return CreateComAdapter(std::move(config), participantName, domainId);
}

auto CreateVAsioComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
{
    if (config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::FastRTPS)
    {
        std::cout << "Creating VAsio ComAdapter but FastRTPS ComAdapter was configured in IbConfig!" << std::endl;
    }
    config.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;
    return CreateComAdapter(std::move(config), participantName, domainId);
}

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
}

