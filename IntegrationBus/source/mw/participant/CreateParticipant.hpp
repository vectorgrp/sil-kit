// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace mw {

auto CreateParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                     const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IParticipantInternal>;

auto ValidateAndSanitizeConfig(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                               const std::string& participantName) -> ib::cfg::ParticipantConfiguration;
} // mw
} // namespace ib

