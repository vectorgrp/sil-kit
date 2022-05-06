// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "MockParticipantConfiguration.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace cfg {

auto MockParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>
{
    return std::make_shared<ib::cfg::ParticipantConfiguration>();
}

auto MockParticipantConfigurationWithLogging(mw::logging::Level logLevel)
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
