// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>

#include "ib/IbMacros.hpp"
#include "ib/exception.hpp"

namespace ib {
namespace cfg {
namespace vasio {

namespace datatypes {
    struct MiddlewareConfiguration;
}

class IMiddlewareConfiguration
{
public:
    virtual ~IMiddlewareConfiguration() = default;

    /*! \brief Convert this configuration into JSON.
    *
    * Create a JSON-formatted string representation from the
    * configuration data object.
    *
    * \return The JSON-formatted string
    */
    virtual auto ToJsonString() -> std::string = 0;

    /*! \brief Convert this configuration into YAML.
    *
    * Create a YAML-formatted string representation from the
    * configuration data object.
    *
    * \return The YAML-formatted string
    */
    virtual auto ToYamlString() -> std::string = 0;
};

/*! \brief Parse configuration from a JSON string.
*
* Create a configuration data object from settings described by a
* JSON string.
*
* \param jsonString A string that adheres to our JSON schema.
* \return The configuration data
*
* \throw ib::cfg::Misconfiguration The input string violates the
* JSON format, schema or an integrity rule.
*/
IntegrationBusAPI auto ReadMiddlewareConfigurationFromJsonString(const std::string& jsonString) -> std::shared_ptr<IMiddlewareConfiguration>;

/*! \brief Parse configuration from a JSON file.
*
* Create a configuration data object from settings described by a
* JSON file.
*
* \param jsonFilename Path to the JSON file.
* \return The configuration data
*
* \throw ib::cfg::Misconfiguration The file could not be read, or
* the input string violates the JSON format, schema or an
* integrity rule.
*/
IntegrationBusAPI auto ReadMiddlewareConfigurationFromJsonFile(const std::string& jsonFilename) -> std::shared_ptr<IMiddlewareConfiguration>;

/*! \brief Parse configuration from a YAML file.
*
* Create a configuration data object from settings described by a
* YAML file.
*
* \param yamlFilename Path to the YAML file.
* \return The configuration data
*
* \throw ib::cfg::Misconfiguration The file could not be read, or
* the input string violates the YAML format, schema or an
* integrity rule.
*/
IntegrationBusAPI auto ReadMiddlewareConfigurationFromYamlFile(const std::string& yamlFilename) -> std::shared_ptr<IMiddlewareConfiguration>;

/*! \brief Parse configuration from a YAML string.
*
* Create a configuration data object from settings described by a
* YAML string.
*
* \param yamlString A string that adheres to our YAML schema.
* \return The configuration data
*
* \throw ib::cfg::Misconfiguration The input string violates the
* YAML format, schema or an integrity rule.
*/
IntegrationBusAPI auto ReadMiddlewareConfigurationFromYamlString(const std::string& yamlString) -> std::shared_ptr<IMiddlewareConfiguration>;

} // namespace vasio
} // namespace cfg
} // namespace ib
