/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig, const std::string& participantName)
    -> std::unique_ptr<IParticipant>;

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
