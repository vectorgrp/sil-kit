// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <thread>

#include "ib/IbMacros.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/IComAdapter.hpp"

namespace ib {

/*! \brief Join the FastRTPS middleware domain as a participant.
* 
* Join the FastRTPS middleware domain and become a participant
* based on the given configuration options.
*
* \param config Configuration of the participant
* \param participantName Name of the participant
* \param fastRtpsDomainId ID of the domain
* \return Instance of the communication adapter
*
* \throw ib::cfg::Misconfiguration if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
* \throw std::exception The FastRTPS participant could not be
* created.
*/
IntegrationBusAPI auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t fastRtpsDomainId) -> std::unique_ptr<mw::IComAdapter>;

} // namespace ib
