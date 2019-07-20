// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CreateComAdapter.hpp"

#include "ComAdapter.hpp"

namespace ib {
namespace mw {

auto CreateFastRtpsComAdapterImpl(ib::cfg::Config config, const std::string& participantName) -> std::unique_ptr<IComAdapterInternal>
{
    return std::make_unique<ComAdapter<FastRtpsConnection>>(std::move(config), participantName);
}

} // mw
} // namespace ib

