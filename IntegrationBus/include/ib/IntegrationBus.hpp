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
* \param fastRtpsDomainId ID of the domain; must be in the range [1, 232]
* \return Instance of the communication adapter
*
* \throw ib::cfg::Misconfiguration if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
* \throw std::exception The FastRTPS participant could not be
* created.
*/
IntegrationBusAPI auto CreateFastRtpsComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t fastRtpsDomainId) -> std::unique_ptr<mw::IComAdapter>;

/*! \brief Join the VAsio middleware domain as a participant.
*
* Join the VAsio middleware domain and become a participant
* based on the given configuration options.
*
* \param config Configuration of the participant
* \param participantName Name of the participant
* \param vAsioDomainId ID of the domain
* \return Instance of the communication adapter
*
* \throw ib::cfg::Misconfiguration if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
*/
IntegrationBusAPI auto CreateVAsioComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t vAsioDomainId) -> std::unique_ptr<mw::IComAdapter>;

/*! \brief Join the configured middleware domain as a participant.
*
* Become a participant based on the the given configuration options.
* The middleware used is selected by the active middleware configuration option.
*
* \param config Configuration of the participant
* \param participantName Name of the participant
* \param domainId ID of the domain; when using FastRTPS, domainId must be in the range [1, 232]
* \return Instance of the communication adapter
*
* \throw ib::cfg::Misconfiguration if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
* \throw std::exception The FastRTPS participant could not be
* created.
*/
IntegrationBusAPI auto CreateComAdapter(ib::cfg::Config config, const std::string& participantName, const uint32_t domainId) -> std::unique_ptr<mw::IComAdapter>;

} // namespace ib
