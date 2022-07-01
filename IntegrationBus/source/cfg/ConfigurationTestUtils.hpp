// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/mw/logging/LoggingDatatypes.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace cfg {

inline auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>;
inline auto MakeParticipantConfigurationWithLogging(mw::logging::Level logLevel) 
    -> std::shared_ptr<ib::cfg::IParticipantConfiguration>;

// inline implementations

auto MakeEmptyParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    return std::make_shared<ib::cfg::ParticipantConfiguration>();
}

auto MakeParticipantConfigurationWithLogging(mw::logging::Level logLevel)
    -> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    ib::cfg::ParticipantConfiguration config;

    auto sink = cfg::Sink{};
    sink.level = logLevel;
    sink.type = cfg::Sink::Type::Stdout;
    config.logging.sinks.push_back(sink);

    return std::make_shared<ib::cfg::ParticipantConfiguration>(std::move(config));
}

} // namespace cfg
} // namespace ib
