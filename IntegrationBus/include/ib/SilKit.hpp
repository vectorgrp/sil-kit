// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <thread>

#include "silkit/SilKitMacros.hpp"
#include "silkit/core/IParticipant.hpp"
#include "silkit/cfg/IParticipantConfiguration.hpp"

namespace SilKit {

/*! \brief Join the configured middleware domain as a participant.
*
* Become a participant based on the the given configuration options.
* The middleware used is selected by the active middleware configuration option.
*
* \param config Configuration of the participant
* \param participantName Name of the participant
* \param registryUri the URI of the registry
* \return Instance of the communication adapter
*
* \throw SilKit::configuration_error if the config has errors
* \throw std::runtime_error Parameter participantName does not name
* a valid participant in the config file.
* \throw std::exception The participant could not be created.
*/

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName)
    -> std::unique_ptr<Core::IParticipant>;

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<Core::IParticipant>;

} // namespace SilKit
