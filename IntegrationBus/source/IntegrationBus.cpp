// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "CreateComAdapter.hpp"

namespace ib {

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
        auto comAdapter = mw::CreateComAdapterImpl(std::move(config), participantName);
        comAdapter->joinIbDomain(domainId);
        return comAdapter;
    }
}

