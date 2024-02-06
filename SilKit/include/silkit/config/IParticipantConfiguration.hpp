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

} // namespace Config
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/config/IParticipantConfiguration.ipp"
//! \endcond
