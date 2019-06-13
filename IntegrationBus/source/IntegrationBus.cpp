// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "ComAdapter.hpp"

namespace ib {

    template <class IbConnectionT>
    auto connect(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
    {
        Validate(config);
        auto comAdapter = std::make_unique<mw::ComAdapter<IbConnectionT>>(std::move(config), participantName);
        comAdapter->joinIbDomain(domainId);

        return std::move(comAdapter);
    }

    auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
    {
        if (config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::VAsio)
        {
            std::cout << "Creating FastRTPS ComAdapter but VAsio ComAdapter was configured in IbConfig!" << std::endl;
        }

        return connect<mw::FastRtpsConnection>(std::move(config), participantName, domainId);
    }

    auto CreateVAsioComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
    {
        if (config.middlewareConfig.activeMiddleware == ib::cfg::Middleware::FastRTPS)
        {
            std::cout << "Creating VAsio ComAdapter but FastRTPS ComAdapter was configured in IbConfig!" << std::endl;
        }

        return connect<mw::VAsioConnection>(std::move(config), participantName, domainId);
    }

    auto CreateComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
    {
        switch (config.middlewareConfig.activeMiddleware)
        {
        // FastRTPS is used as default if unconfigured
        case ib::cfg::Middleware::NotConfigured: 
        case ib::cfg::Middleware::FastRTPS:
            return connect<mw::FastRtpsConnection>(std::move(config), participantName, domainId);

        case ib::cfg::Middleware::VAsio:
            return connect<mw::VAsioConnection>(std::move(config), participantName, domainId);

        default:
            throw ib::cfg::Misconfiguration{ "Unknown active middleware selected" };
        }
    }
}

