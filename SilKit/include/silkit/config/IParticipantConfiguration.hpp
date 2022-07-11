// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>

#include "silkit/SilKitMacros.hpp"
#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Config {

class IParticipantConfiguration
{
public:
    virtual ~IParticipantConfiguration() = default;
};

/*! \brief Parse configuration from a YAML or JSON string.
*
* Create a configuration data object from settings described by a
* YAML or JSON formatted string.
*
* \param text A string that adheres to our JSON schema.
* \return The configuration data
*
* \throw SilKit::configuration_error The input string violates the
* JSON format, schema or an integrity rule.
*/
SilKitAPI auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

/*! \brief Parse configuration from a YAML or JSON file.
*
* Create a configuration data object from settings described by a
* YAML or JSON file.
*
* \param filename Path to the YAML or JSON file.
* \return The configuration data
*
* \throw SilKit::configuration_error The file could not be read, or
* the input string violates the YAML/JSON format, schema or an
* integrity rule.
*/
SilKitAPI auto ParticipantConfigurationFromFile(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

} // namespace Config
} // namespace SilKit
