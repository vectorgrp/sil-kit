// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapterInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace mw {

auto CreateSimulationParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                     const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IComAdapterInternal>;

auto ValidateAndSanitizeConfig(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> ib::cfg::datatypes::ParticipantConfiguration;
} // mw
} // namespace ib

