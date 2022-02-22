// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/cfg/IParticipantConfiguration.hpp"

namespace ib {
namespace cfg {

auto MockParticipantConfiguration() -> std::shared_ptr<ib::cfg::IParticipantConfiguration>;

} // namespace cfg
} // namespace ib
