// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

// public include
#include "ib/mw/sync/SyncDatatypes.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {
//! The synchronization namespace
namespace sync {

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
};

} // namespace sync
} // namespace mw
} // namespace ib
