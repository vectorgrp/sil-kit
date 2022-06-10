// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IParticipantInternal.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"

namespace ib {
namespace mw {

auto CreateNullConnectionParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                        const std::string& participantName)
    -> std::unique_ptr<IParticipantInternal>;

} // namespace mw
} // namespace ib

