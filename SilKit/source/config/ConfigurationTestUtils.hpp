// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Config {

inline auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;
inline auto MakeParticipantConfigurationWithLogging(Services::Logging::Level logLevel) 
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>;

// inline implementations

auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return std::make_shared<SilKit::Config::ParticipantConfiguration>();
}

auto MakeParticipantConfigurationWithLogging(Services::Logging::Level logLevel)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    SilKit::Config::ParticipantConfiguration config;

    auto sink = Config::Sink{};
    sink.level = logLevel;
    sink.type = Config::Sink::Type::Stdout;
    config.logging.sinks.push_back(sink);

    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(config));
}

} // namespace Config
} // namespace SilKit
