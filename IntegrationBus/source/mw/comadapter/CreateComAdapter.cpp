// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CreateComAdapter.hpp"

#include "ComAdapter.hpp"
#include <iostream>

namespace ib {
namespace mw {

auto CreateVAsioSimulationParticipantImpl(cfg::datatypes::ParticipantConfiguration participantConfig,
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
    auto&& cfg = ValidateAndSanitizeConfig(participantConfig, participantName);

    return CreateVAsioSimulationParticipantImpl(std::move(cfg), participantName, isSynchronized);
}

auto ValidateAndSanitizeConfig(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> cfg::datatypes::ParticipantConfiguration
{
    if (participantName.empty())
    {
        throw ib::ConfigurationError("An empty participant name is not allowed");
    }

    // try to cast to ParticipantConfiguration to check if the shared pointer is valid
    auto cfg = std::dynamic_pointer_cast<cfg::datatypes::ParticipantConfiguration>(participantConfig);
    if (cfg == nullptr)
    {
        return {};
    }
    return *cfg;
}

} // mw
} // namespace ib
