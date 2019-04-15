// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "FastRtpsComAdapter.hpp"


namespace ib {
    auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t fastRtpsDomainId) -> std::unique_ptr<mw::IComAdapter>
    {
        Validate(config);
        auto fastRtpsComAdapter = std::make_unique<mw::FastRtpsComAdapter>(std::move(config), participantName);
        fastRtpsComAdapter->joinIbDomain(fastRtpsDomainId);

        return std::move(fastRtpsComAdapter);
    }
}

