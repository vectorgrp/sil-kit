// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "TimeSyncServiceExtensionsImpl.hpp"

#include "TimeSyncService.hpp"

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {

auto AddExternalCouplingHandler(SilKit::Services::Orchestration::ITimeSyncService* iTimeSyncService,
                                std::function<void()> handler) -> SilKit::Util::HandlerId
{
    const auto timeSyncService = static_cast<SilKit::Services::Orchestration::TimeSyncService*>(iTimeSyncService);
    const auto handlerId = timeSyncService->AddExternalCouplingHandler(handler);
    return handlerId;
}

void RemoveExternalCouplingHandler(SilKit::Services::Orchestration::ITimeSyncService* iTimeSyncService,
                                   SilKit::Util::HandlerId handlerId)
{
    const auto timeSyncService = static_cast<SilKit::Services::Orchestration::TimeSyncService*>(iTimeSyncService);
    timeSyncService->RemoveExternalCouplingHandler(handlerId);
}

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit
