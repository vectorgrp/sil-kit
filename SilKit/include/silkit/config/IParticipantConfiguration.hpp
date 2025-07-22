// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <memory>

#include "silkit/detail/macros.hpp"


namespace SilKit {
namespace Config {

class IParticipantConfiguration
{
public:
    virtual ~IParticipantConfiguration() = default;
};

} // namespace Config
} // namespace SilKit


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Config {

/*! \brief Parse configuration from a YAML or JSON string.
 *
 * Create a configuration data object from settings described by a
 * YAML or JSON formatted string.
 *
 * \param text A string that adheres to our JSON schema (UTF-8).
 * \return The configuration data
 *
 * \throw SilKit::ConfigurationError The input string violates the
 * JSON format, schema or an integrity rule.
 */
DETAIL_SILKIT_CPP_API auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

/*! \brief Parse configuration from a YAML or JSON file.
 *
 * Create a configuration data object from settings described by a
 * YAML or JSON file.
 *
 * \param filename Path to the YAML or JSON file (UTF-8).
 * \return The configuration data
 *
 * \throw SilKit::ConfigurationError The file could not be read, or
 * the input string violates the YAML/JSON format, schema or an
 * integrity rule.
 */
DETAIL_SILKIT_CPP_API auto ParticipantConfigurationFromFile(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

//! \brief Returns the parsed configuration with all includes processed.
DETAIL_SILKIT_CPP_API auto ParticipantConfigurationToJson(
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> config) -> std::string;

} // namespace Config
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/config/IParticipantConfiguration.ipp"
//! \endcond
