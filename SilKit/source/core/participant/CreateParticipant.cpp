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

#include <iostream>

#include "CreateParticipant.hpp"
#include "CreateParticipant_impl.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                           const std::string& participantName) -> std::unique_ptr<IParticipantInternal>
{
    return CreateParticipantImplInternal<VAsioConnection>(std::move(participantConfig), participantName);
}

auto ValidateAndSanitizeConfig(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig)
    -> Config::ParticipantConfiguration
{
    // try to cast to ParticipantConfiguration to check if the shared pointer is valid
    auto cfg = std::dynamic_pointer_cast<Config::ParticipantConfiguration>(participantConfig);
    if (cfg == nullptr)
    {
        return {};
    }

    return *cfg;
}

auto ValidateAndSanitizeParticipantName(Config::ParticipantConfiguration participantConfig,
                                        const std::string& participantName) -> ParticipantNameValidationResult
{
    if (participantName.empty())
    {
        throw SilKit::ConfigurationError("An empty participant name is not allowed");
    }

    ParticipantNameValidationResult result{participantName, {}};

    if (!participantConfig.participantName.empty() && participantConfig.participantName != result.participantName)
    {
        result.participantName = participantConfig.participantName;
        result.logMessages.emplace_back(
            Services::Logging::Level::Info,
            fmt::format("The provided participant name '{}' differs from the configured name '{}'. The latter will be "
                        "used.",
                        participantName, participantConfig.participantName));
    }

    return result;
}

} // namespace Core
} // namespace SilKit