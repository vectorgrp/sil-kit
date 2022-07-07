// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"

namespace SilKit {
namespace Core {

auto CreateNullConnectionParticipantImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                        const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>;

} // namespace Core
} // namespace SilKit

