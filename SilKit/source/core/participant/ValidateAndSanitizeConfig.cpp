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

#include "ValidateAndSanitizeConfig.hpp"
#include "Assert.hpp"

#include <fmt/format.h>

namespace SilKit {
namespace Core {

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName, const std::string& registryUri)
    -> ValidateAndSanitizeConfigResult
{
    ValidateAndSanitizeConfigResult result;

    if (participantConfig == nullptr)
    {
        result.logMessages.emplace_back(Services::Logging::Level::Warn,
                                        "The participant configuration was not specified.");
    }
    else
    {
        // make sure the participant configuration has the correct type
        auto cfg = std::dynamic_pointer_cast<Config::ParticipantConfiguration>(participantConfig);
        SILKIT_ASSERT(cfg != nullptr);

        result.participantConfiguration = *cfg;
    }

    // ==================================================================================
    // Participant Name
    // ==================================================================================

    if (participantName.empty())
    {
        throw SilKit::ConfigurationError("An empty participant name is not allowed");
    }

    if (result.participantConfiguration.participantName.empty())
    {
        result.participantConfiguration.participantName = participantName;
    }
    else if (result.participantConfiguration.participantName != participantName)
    {
        result.logMessages.emplace_back(
            Services::Logging::Level::Info,
            fmt::format("The provided participant name '{}' differs from the configured name '{}'. The latter will be "
                        "used.",
                        participantName, result.participantConfiguration.participantName));
    }

    // ==================================================================================
    // Registry URI
    // ==================================================================================

    if (result.participantConfiguration.middleware.registryUri.empty())
    {
        if (registryUri.empty())
        {
            result.participantConfiguration.middleware.registryUri = "silkit://localhost:8500";

            result.logMessages.emplace_back(
                Services::Logging::Level::Warn,
                fmt::format(
                    "No registry URI was specified in the configuration nor was one provided. Using the default '{}'.",
                    result.participantConfiguration.middleware.registryUri));
        }
        else
        {
            result.participantConfiguration.middleware.registryUri = registryUri;
        }
    }
    else if (result.participantConfiguration.middleware.registryUri != registryUri)
    {
        result.logMessages.emplace_back(
            Services::Logging::Level::Info,
            fmt::format(
                "The provided registry URI '{}' differs from the configured registry URI '{}'. The latter will be "
                "used.",
                registryUri, result.participantConfiguration.middleware.registryUri));
    }

    return result;
}

} // namespace Core
} // namespace SilKit
