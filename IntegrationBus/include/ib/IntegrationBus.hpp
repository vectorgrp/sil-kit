// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <thread>

#include "ib/IbMacros.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"

namespace ib {

/*! \brief Join the configured middleware domain as a participant.
*
* Become a participant based on the the given configuration options.
* The middleware used is selected by the active middleware configuration option.
*
* \param config Configuration of the participant
* \param participantName Name of the participant
* \param isSynchronized Simulation time synchronization
* \param domainId ID of the domain
* \return Instance of the communication adapter
*
* \throw ib::cfg::Misconfiguration if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
* \throw std::exception The participant could not be created.
*/
IntegrationBusAPI auto CreateSimulationParticipant(
    std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig, const std::string& participantName,
    const uint32_t domainId, bool isSynchronized) -> std::unique_ptr<mw::IComAdapter>;

} // namespace ib
