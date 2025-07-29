// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

inline auto MakeParticipantConfigurationStringWithLogging(Services::Logging::Level logLevel) -> std::string;

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

auto MakeParticipantConfigurationStringWithLogging(Services::Logging::Level logLevel) -> std::string
{
    std::ostringstream ss;
    ss << R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level": ")";
    ss << to_string(logLevel);
    ss << R"("}]}})";

    return ss.str();
}

auto MakeParticipantConfigurationWithLogging(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configString = MakeParticipantConfigurationStringWithLogging(logLevel);
    return SilKit::Config::ParticipantConfigurationFromString(configString);
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
