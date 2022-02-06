// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapterInternal.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"

namespace ib {
namespace mw {

auto CreateVAsioComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;
auto CreateComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;
auto CreateSimulationParticipantImpl(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                     const std::string& participantName, cfg::Config config)
    -> std::unique_ptr<IComAdapterInternal>;

} // mw
} // namespace ib

