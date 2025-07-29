// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/SilKitMacros.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN

/*! \brief Create a participant and join the simulation.
 *
 * Become a participant based on the the given configuration options.
 *
 * \param participantConfig Configuration of the participant
 * \param participantName Name of the participant (UTF-8)
 * \return Instance of the communication adapter
 *
 * \throw SilKit::ConfigurationError if the config has errors
 * \throw SilKit::SilKitError The participant could not be created.
 */
DETAIL_SILKIT_CPP_API auto CreateParticipant(
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
    const std::string& participantName) -> std::unique_ptr<IParticipant>;

/*! \brief Create a participant and join the simulation.
 *
 * Become a participant based on the the given configuration options.
 *
 * \param participantConfig Configuration of the participant
 * \param participantName Name of the participant (UTF-8)
 * \param registryUri The URI of the registry
 * \return Instance of the communication adapter
 *
 * \throw SilKit::ConfigurationError if the config has errors
 * \throw SilKit::SilKitError The participant could not be created.
 */
DETAIL_SILKIT_CPP_API auto CreateParticipant(
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig, const std::string& participantName,
    const std::string& registryUri) -> std::unique_ptr<IParticipant>;

DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/SilKit.ipp"
//! \endcond
