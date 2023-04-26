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

#include <memory>
#include <sstream>

#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/logging/string_utils.hpp"

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Config {

inline auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

inline auto MakeParticipantConfigurationWithLogging(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

inline auto MakeEmptyParticipantConfigurationImpl() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>;

inline auto MakeParticipantConfigurationWithLoggingImpl(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>;

} // namespace Config
} // namespace SilKit

// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
namespace Config {

auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return SilKit::Config::ParticipantConfigurationFromString("");
}

auto MakeParticipantConfigurationWithLogging(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    std::ostringstream ss;
    ss << R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level": ")";
    ss << to_string(logLevel);
    ss << R"("}]}})";

    return SilKit::Config::ParticipantConfigurationFromString(ss.str());
}

inline auto MakeEmptyParticipantConfigurationImpl() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
{
    return std::make_shared<SilKit::Config::ParticipantConfiguration>();
}

inline auto MakeParticipantConfigurationWithLoggingImpl(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
{
    auto participantConfiguration = std::make_shared<SilKit::Config::ParticipantConfiguration>();

    SilKit::Config::Sink sink{};
    sink.type = Sink::Type::Stdout;
    sink.level = logLevel;
    participantConfiguration->logging.sinks.emplace_back(sink);

    return participantConfiguration;
}

} // namespace Config
} // namespace SilKit
