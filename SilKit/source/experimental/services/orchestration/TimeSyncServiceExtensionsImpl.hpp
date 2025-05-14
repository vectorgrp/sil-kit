// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <cstdint>

namespace SilKit {
namespace Services {
namespace Orchestration {
class ITimeSyncService;
} // namespace Orchestration
} // namespace Services
} // namespace SilKit

namespace SilKit {
namespace Util {
enum struct HandlerId : std::uint64_t;
} // namespace Util
} // namespace SilKit

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {

auto AddOtherSimulationStepsCompletedHandler(SilKit::Services::Orchestration::ITimeSyncService* timeSyncService,
                                             std::function<void()> handler) -> SilKit::Util::HandlerId;

void RemoveOtherSimulationStepsCompletedHandler(SilKit::Services::Orchestration::ITimeSyncService* timeSyncService,
                                                SilKit::Util::HandlerId handlerId);

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit
