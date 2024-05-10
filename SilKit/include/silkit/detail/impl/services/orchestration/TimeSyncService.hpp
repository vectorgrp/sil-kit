// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Orchestration {

class TimeSyncService : public SilKit::Services::Orchestration::ITimeSyncService
{
public:
    inline explicit TimeSyncService(SilKit_LifecycleService* lifecycleService);

    inline ~TimeSyncService() override = default;

    inline void SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override;

    inline void SetSimulationStepHandlerAsync(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override;

    inline void CompleteSimulationStep() override;

    inline auto Now() const -> std::chrono::nanoseconds override;

private:
    SilKit_TimeSyncService* _timeSyncService{nullptr};

    std::unique_ptr<SimulationStepHandler> _simulationStepHandler;
    std::unique_ptr<SimulationStepHandler> _simulationStepHandlerAsync;
};

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Orchestration {

TimeSyncService::TimeSyncService(SilKit_LifecycleService* lifecycleService)
{
    const auto returnCode = SilKit_TimeSyncService_Create(&_timeSyncService, lifecycleService);
    ThrowOnError(returnCode);
}

void TimeSyncService::SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize)
{
    auto ownedHandlerPtr = std::make_unique<SimulationStepHandler>(std::move(task));

    const auto cSimulationStepHandler = [](void* context, SilKit_TimeSyncService* timeSyncService,
                                           SilKit_NanosecondsTime now, SilKit_NanosecondsTime duration) {
        SILKIT_UNUSED_ARG(timeSyncService);

        const auto handlerPtr = static_cast<SimulationStepHandler*>(context);
        (*handlerPtr)(std::chrono::nanoseconds{now}, std::chrono::nanoseconds{duration});
    };

    const auto returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
        _timeSyncService, ownedHandlerPtr.get(), cSimulationStepHandler, initialStepSize.count());
    ThrowOnError(returnCode);

    _simulationStepHandler = std::move(ownedHandlerPtr);
}

void TimeSyncService::SetSimulationStepHandlerAsync(SimulationStepHandler task,
                                                    std::chrono::nanoseconds initialStepSize)
{
    auto ownedHandlerPtr = std::make_unique<SimulationStepHandler>(std::move(task));

    const auto cSimulationStepHandler = [](void* context, SilKit_TimeSyncService* timeSyncService,
                                           SilKit_NanosecondsTime now, SilKit_NanosecondsTime duration) {
        SILKIT_UNUSED_ARG(timeSyncService);

        const auto handlerPtr = static_cast<SimulationStepHandler*>(context);
        (*handlerPtr)(std::chrono::nanoseconds{now}, std::chrono::nanoseconds{duration});
    };

    const auto returnCode = SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
        _timeSyncService, ownedHandlerPtr.get(), cSimulationStepHandler, initialStepSize.count());
    ThrowOnError(returnCode);

    _simulationStepHandlerAsync = std::move(ownedHandlerPtr);
}

void TimeSyncService::CompleteSimulationStep()
{
    const auto returnCode = SilKit_TimeSyncService_CompleteSimulationStep(_timeSyncService);
    ThrowOnError(returnCode);
}

auto TimeSyncService::Now() const -> std::chrono::nanoseconds
{
    SilKit_NanosecondsTime nanosecondsTime;

    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_TimeSyncService_Now(_timeSyncService, &nanosecondsTime);
    ThrowOnError(returnCode);

    return std::chrono::nanoseconds{nanosecondsTime};
}

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
