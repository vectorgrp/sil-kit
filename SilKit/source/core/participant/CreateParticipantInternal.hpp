// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Core {

auto CreateParticipantInternal(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                               const std::string& participantName,
                               const std::string& registryUri) -> std::unique_ptr<IParticipantInternal>;

} // namespace Core
} // namespace SilKit