#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"

#include "silkit/hourglass/impl/Macros.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Orchestration {

class TimeSyncService : public SilKit::Services::Orchestration::ITimeSyncService
{
public:
    explicit TimeSyncService(SilKit_LifecycleService* lifecycleService)
    {
        const auto returnCode = SilKit_TimeSyncService_Create(&_timeSyncService, lifecycleService);
        ThrowOnError(returnCode);
    }

    ~TimeSyncService() override = default;

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Orchestration::TimeSyncService", FUNCTION_NAME)

    void SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override
    {
        _simulationStepHandler = task;

        const auto cSimulationStepHandler = [](void* context, SilKit_TimeSyncService* timeSyncService,
                                               SilKit_NanosecondsTime now, SilKit_NanosecondsTime duration) {
            SILKIT_UNUSED_ARG(timeSyncService);
            static_cast<TimeSyncService*>(context)->_simulationStepHandler(std::chrono::nanoseconds{now},
                                                                           std::chrono::nanoseconds{duration});
        };

        const auto returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(
            _timeSyncService, this, cSimulationStepHandler, initialStepSize.count());
        ThrowOnError(returnCode);
    }

    void SetSimulationStepHandlerAsync(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override
    {
        _simulationStepHandlerAsync = task;

        const auto cSimulationStepHandler = [](void* context, SilKit_TimeSyncService* timeSyncService,
                                               SilKit_NanosecondsTime now, SilKit_NanosecondsTime duration) {
            SILKIT_UNUSED_ARG(timeSyncService);
            static_cast<TimeSyncService*>(context)->_simulationStepHandlerAsync(std::chrono::nanoseconds{now},
                                                                                std::chrono::nanoseconds{duration});
        };

        const auto returnCode = SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
            _timeSyncService, this, cSimulationStepHandler, initialStepSize.count());
        ThrowOnError(returnCode);
    }

    void CompleteSimulationStep() override
    {
        const auto returnCode = SilKit_TimeSyncService_CompleteSimulationStep(_timeSyncService);
        ThrowOnError(returnCode);
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Now");
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

private:
    SilKit_TimeSyncService* _timeSyncService{nullptr};

    SimulationStepHandler _simulationStepHandler;
    SimulationStepHandler _simulationStepHandlerAsync;
};

} // namespace Orchestration
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
