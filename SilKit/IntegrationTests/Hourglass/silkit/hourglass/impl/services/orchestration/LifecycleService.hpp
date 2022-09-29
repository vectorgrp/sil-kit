#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ILifecycleService.hpp"

#include "silkit/hourglass/impl/Macros.hpp"
#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/services/orchestration/TimeSyncService.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Orchestration {

using SilKit::Services::Orchestration::ParticipantState;

class LifecycleService : public SilKit::Services::Orchestration::ILifecycleService
{
public:
    explicit LifecycleService(SilKit_Participant *participant,
                              SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
    {
        SilKit_LifecycleConfiguration lifecycleConfiguration;
        SilKit_Struct_Init(SilKit_LifecycleConfiguration, lifecycleConfiguration);
        lifecycleConfiguration.operationMode = static_cast<SilKit_OperationMode>(startConfiguration.operationMode);

        const auto returnCode =
            SilKit_LifecycleService_Create(&_lifecycleService, participant, &lifecycleConfiguration);
        ThrowOnError(returnCode);
    }

    ~LifecycleService() override = default;

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Orchestration::LifecycleService", FUNCTION_NAME)

    void SetCommunicationReadyHandler(SilKit::Services::Orchestration::CommunicationReadyHandler handler) override
    {
        _communicationReadyHandler = std::move(handler);

        const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
            SILKIT_UNUSED_ARG(lifecycleService);
            static_cast<LifecycleService *>(context)->_communicationReadyHandler();
        };

        const auto returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(_lifecycleService, this, cHandler);
        ThrowOnError(returnCode);
    }

    void SetCommunicationReadyHandlerAsync(SilKit::Services::Orchestration::CommunicationReadyHandler handler) override
    {
        _communicationReadyHandlerAsync = std::move(handler);

        const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
            SILKIT_UNUSED_ARG(lifecycleService);
            static_cast<LifecycleService *>(context)->_communicationReadyHandlerAsync();
        };

        const auto returnCode =
            SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(_lifecycleService, this, cHandler);
        ThrowOnError(returnCode);
    }

    void CompleteCommunicationReadyHandlerAsync() override
    {
        const auto returnCode = SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(_lifecycleService);
        ThrowOnError(returnCode);
    }

    void SetStartingHandler(SilKit::Services::Orchestration::StartingHandler handler) override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("SetStartingHandler");
    }

    void SetStopHandler(SilKit::Services::Orchestration::StopHandler handler) override
    {
        _stopHandler = handler;

        const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
            SILKIT_UNUSED_ARG(lifecycleService);
            static_cast<LifecycleService *>(context)->_stopHandler();
        };

        const auto returnCode = SilKit_LifecycleService_SetStopHandler(_lifecycleService, this, cHandler);
        ThrowOnError(returnCode);
    }

    void SetShutdownHandler(SilKit::Services::Orchestration::ShutdownHandler handler) override
    {
        _shutdownHandler = handler;

        const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
            SILKIT_UNUSED_ARG(lifecycleService);
            static_cast<LifecycleService *>(context)->_shutdownHandler();
        };

        const auto returnCode = SilKit_LifecycleService_SetShutdownHandler(_lifecycleService, this, cHandler);
        ThrowOnError(returnCode);
    }

    void SetAbortHandler(SilKit::Services::Orchestration::AbortHandler handler) override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("SetAbortHandler");
    }

    auto StartLifecycle() -> std::future<ParticipantState> override
    {
        const auto returnCode = SilKit_LifecycleService_StartLifecycle(_lifecycleService);
        ThrowOnError(returnCode);

        std::promise<ParticipantState> participantStatePromise;
        auto participantStateFuture = participantStatePromise.get_future();

        std::thread waiter{[this, promise = std::move(participantStatePromise)]() mutable {
            SilKit_ParticipantState participantState;

            try
            {
                const auto returnCode =
                    SilKit_LifecycleService_WaitForLifecycleToComplete(_lifecycleService, &participantState);
                ThrowOnError(returnCode);
            }
            catch (...)
            {
                promise.set_exception(std::current_exception());
                return;
            }

            promise.set_value(static_cast<SilKit::Services::Orchestration::ParticipantState>(participantState));
        }};
        waiter.detach();

        return participantStateFuture;
    }

    void ReportError(std::string errorMsg) override
    {
        SILKIT_UNUSED_ARG(errorMsg);
        SILKIT_THROW_NOT_IMPLEMENTED_("ReportError");
    }

    void Pause(std::string reason) override
    {
        SILKIT_UNUSED_ARG(reason);
        SILKIT_THROW_NOT_IMPLEMENTED_("Pause");
    }

    void Continue() override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Continue");
    }

    void Stop(std::string reason) override
    {
        const auto returnCode = SilKit_LifecycleService_Stop(_lifecycleService, reason.c_str());
        ThrowOnError(returnCode);
    }

    auto State() const -> SilKit::Services::Orchestration::ParticipantState override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("State");
    }

    auto Status() const -> SilKit::Services::Orchestration::ParticipantStatus const & override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Status");
    }

    auto CreateTimeSyncService() -> SilKit::Services::Orchestration::ITimeSyncService * override
    {
        _timeSyncService =
            std::make_unique<SilKit::Hourglass::Impl::Services::Orchestration::TimeSyncService>(_lifecycleService);

        return _timeSyncService.get();
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

private:
    SilKit_LifecycleService *_lifecycleService{nullptr};

    SilKit::Services::Orchestration::CommunicationReadyHandler _communicationReadyHandler;
    SilKit::Services::Orchestration::CommunicationReadyHandler _communicationReadyHandlerAsync;
    SilKit::Services::Orchestration::StopHandler _stopHandler;
    SilKit::Services::Orchestration::ShutdownHandler _shutdownHandler;

    std::unique_ptr<SilKit::Hourglass::Impl::Services::Orchestration::TimeSyncService> _timeSyncService;
};

} // namespace Orchestration
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
