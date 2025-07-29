// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"
#include "silkit/experimental/services/orchestration/TimeSyncDatatypesExtensions.hpp"

#include <unordered_map>


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

    inline void SetSimulationStepHandlerAsync(SimulationStepHandler task,
                                              std::chrono::nanoseconds initialStepSize) override;

    inline void CompleteSimulationStep() override;

    inline auto Now() const -> std::chrono::nanoseconds override;

public:
    inline auto ExperimentalAddOtherSimulationStepsCompletedHandler(
        SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler) -> SilKit::Util::HandlerId;

    inline void ExperimentalRemoveOtherSimulationStepsCompletedHandler(SilKit::Util::HandlerId handlerId);

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_TimeSyncService* _timeSyncService{nullptr};

    std::unique_ptr<SimulationStepHandler> _simulationStepHandler;
    std::unique_ptr<SimulationStepHandler> _simulationStepHandlerAsync;
    HandlerDataMap<SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler>
        _otherSimulationStepsCompletedHandlers;
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

    const auto returnCode = SilKit_TimeSyncService_Now(_timeSyncService, &nanosecondsTime);
    ThrowOnError(returnCode);

    return std::chrono::nanoseconds{nanosecondsTime};
}

inline auto TimeSyncService::ExperimentalAddOtherSimulationStepsCompletedHandler(std::function<void()> handler)
    -> SilKit::Util::HandlerId
{
    const auto cHandler = [](void* context, SilKit_TimeSyncService* cTimeSyncService) {
        SILKIT_UNUSED_ARG(cTimeSyncService);

        const auto data = static_cast<
            HandlerData<SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler>*>(context);
        data->handler();
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<
        HandlerData<SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler>>();
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_Experimental_TimeSyncService_AddOtherSimulationStepsCompletedHandler(
        _timeSyncService, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _otherSimulationStepsCompletedHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId),
                                                   std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

inline void TimeSyncService::ExperimentalRemoveOtherSimulationStepsCompletedHandler(
    const SilKit::Util::HandlerId cppHandlerId)
{
    const auto cHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);

    const auto returnCode =
        SilKit_Experimental_TimeSyncService_RemoveOtherSimulationStepsCompletedHandler(_timeSyncService, cHandlerId);
    ThrowOnError(returnCode);
}

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
