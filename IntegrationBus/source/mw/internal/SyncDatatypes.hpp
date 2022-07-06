// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "silkit/core/sync/SyncDatatypes.hpp"

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
