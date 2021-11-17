// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IComAdapterInternal.hpp"
#include "ib/cfg/Config.hpp"

namespace ib {
namespace mw {

auto CreateFastRtpsComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;
auto CreateVAsioComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;
auto CreateComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>;

} // mw
} // namespace ib

