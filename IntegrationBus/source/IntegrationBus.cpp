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
        return connect<mw::FastRtpsConnection>(std::move(config), participantName, domainId);
    }
}

