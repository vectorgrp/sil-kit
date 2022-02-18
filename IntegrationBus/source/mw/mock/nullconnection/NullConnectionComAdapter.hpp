// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapterInternal.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"

namespace ib {
namespace mw {

auto CreateNullConnectionComAdapterImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                        const std::string& participantName, bool isSynchronized)
    -> std::unique_ptr<IComAdapterInternal>;

} // mw
} // namespace ib

