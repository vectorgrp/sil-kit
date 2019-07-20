// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "IntegrationBus.hpp"

#include "Validation.hpp"

#include "CreateComAdapter.hpp"

namespace ib {

    auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>
    {
        Validate(config);
        auto comAdapter = mw::CreateFastRtpsComAdapterImpl(std::move(config), participantName);
        comAdapter->joinIbDomain(domainId);

        return comAdapter;
    }

}

