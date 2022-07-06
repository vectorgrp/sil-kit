// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "silkit/services/orchestration/SyncDatatypes.hpp"

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
