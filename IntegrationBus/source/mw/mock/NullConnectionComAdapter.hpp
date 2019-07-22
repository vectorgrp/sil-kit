// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapter_internal.hpp"
#include "ib/cfg/Config.hpp"

namespace ib {
namespace mw {

auto CreateNullConnectionComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;

} // mw
} // namespace ib

