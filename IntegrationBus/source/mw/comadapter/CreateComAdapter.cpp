// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CreateComAdapter.hpp"

#include "ComAdapter.hpp"
#include <iostream>

namespace ib {
namespace mw {

auto CreateVAsioSimulationParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                          const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IComAdapterInternal>
{
#if defined(IB_MW_HAVE_VASIO)
    return std::make_unique<ComAdapter<VAsioConnection>>(std::move(participantConfig), participantName, isSynchronized);
#else
    std::cout << "ERROR: CreateVasioComAdapterImpl(): IntegrationBus was compiled without \"IB_MW_HAVE_VASIO\""
              << std::endl;
    throw std::runtime_error("VIB was compiled without IB_MW_HAVE_VASIO");
#endif
}

auto CreateSimulationParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                     const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IComAdapterInternal>
{
    return CreateVAsioSimulationParticipantImpl(std::move(participantConfig), participantName, isSynchronized);
}

} // mw
} // namespace ib
