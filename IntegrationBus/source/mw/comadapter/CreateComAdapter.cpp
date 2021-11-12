// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CreateComAdapter.hpp"

#include "ComAdapter.hpp"
#include <iostream>

namespace ib {
namespace mw {

auto CreateFastRtpsComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
#if defined(IB_MW_HAVE_FASTRTPS)
    return std::make_unique<ComAdapter<FastRtpsConnection>>(std::move(config), participantName);
#else
    std::cout << "ERROR: CreateFastRtpsComAdapterImpl(): IntegrationBus was compiled without \"IB_MW_HAVE_FASTRPTS\"" << std::endl;
    throw std::runtime_error("VIB was compiled without IB_MW_HAVE_FASTRTPS");
#endif
}
auto CreateVAsioComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
#if defined(IB_MW_HAVE_VASIO)
    return std::make_unique<ComAdapter<VAsioConnection>>(std::move(config), participantName);
#else
    std::cout << "ERROR: CreateVasioComAdapterImpl(): IntegrationBus was compiled without \"IB_MW_HAVE_VASIO\"" << std::endl;
    throw std::runtime_error("VIB was compiled without IB_MW_HAVE_VASIO");
#endif
}

auto CreateComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    switch (config.middlewareConfig.activeMiddleware)
    {
    // FastRTPS is used as default if unconfigured
    case ib::cfg::Middleware::NotConfigured:
    // [[fallthrough]];

    case ib::cfg::Middleware::VAsio:
        return CreateVAsioComAdapterImpl(std::move(config), participantName);
    
    case ib::cfg::Middleware::FastRTPS:
        std::cout <<"WARNING: FastRTPS is deprecated" << std::endl;
        return CreateFastRtpsComAdapterImpl(std::move(config), participantName);

    default:
        throw ib::cfg::Misconfiguration{ "Unknown active middleware selected" };
    }
}

} // mw
} // namespace ib

