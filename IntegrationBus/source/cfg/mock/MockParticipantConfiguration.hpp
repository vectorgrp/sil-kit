// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/cfg/IParticipantConfiguration.hpp"
#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace cfg {

auto MockParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>;
auto MockParticipantConfigurationWithLogging(mw::logging::Level logLevel) 
    -> std::shared_ptr<ib::cfg::IParticipantConfiguration>;

} // namespace cfg
} // namespace ib
