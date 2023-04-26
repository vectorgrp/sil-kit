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
#include "silkit/services/orchestration/ILifecycleService.hpp"

#include "silkit/detail/impl/services/orchestration/TimeSyncService.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Orchestration {

using SilKit::Services::Orchestration::ParticipantState;

class LifecycleService : public SilKit::Services::Orchestration::ILifecycleService
{
    using StartingHandler = SilKit::Services::Orchestration::StartingHandler;
    using CommunicationReadyHandler = SilKit::Services::Orchestration::CommunicationReadyHandler;
    using StopHandler = SilKit::Services::Orchestration::StopHandler;
    using ShutdownHandler = SilKit::Services::Orchestration::ShutdownHandler;
    using AbortHandler = SilKit::Services::Orchestration::AbortHandler;

public:
    inline explicit LifecycleService(SilKit_Participant *participant,
                              SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration);

    inline ~LifecycleService() override = default;

    inline void SetCommunicationReadyHandler(SilKit::Services::Orchestration::CommunicationReadyHandler handler) override;

    inline void SetCommunicationReadyHandlerAsync(SilKit::Services::Orchestration::CommunicationReadyHandler handler) override;

    inline void CompleteCommunicationReadyHandlerAsync() override;

    inline void SetStartingHandler(SilKit::Services::Orchestration::StartingHandler handler) override;

    inline void SetStopHandler(SilKit::Services::Orchestration::StopHandler handler) override;

    inline void SetShutdownHandler(SilKit::Services::Orchestration::ShutdownHandler handler) override;

    inline void SetAbortHandler(SilKit::Services::Orchestration::AbortHandler handler) override;

    inline auto StartLifecycle() -> std::future<ParticipantState> override;

    inline void ReportError(std::string errorMsg) override;

    inline void Pause(std::string reason) override;

    inline void Continue() override;

    inline void Stop(std::string reason) override;

    inline auto State() const -> SilKit::Services::Orchestration::ParticipantState override;

    inline auto Status() const -> SilKit::Services::Orchestration::ParticipantStatus const & override;

    inline auto CreateTimeSyncService() -> SilKit::Services::Orchestration::ITimeSyncService * override;

private:
    SilKit_LifecycleService *_lifecycleService{nullptr};

    std::unique_ptr<StartingHandler> _startingHandler;
    std::unique_ptr<CommunicationReadyHandler> _communicationReadyHandler;
    std::unique_ptr<CommunicationReadyHandler> _communicationReadyHandlerAsync;
    std::unique_ptr<StopHandler> _stopHandler;
    std::unique_ptr<ShutdownHandler> _shutdownHandler;
    std::unique_ptr<AbortHandler> _abortHandler;

    std::unique_ptr<TimeSyncService> _timeSyncService;

    mutable SilKit::Services::Orchestration::ParticipantStatus _lastParticipantStatus;
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

LifecycleService::LifecycleService(SilKit_Participant *participant,
                                   SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
{
    SilKit_LifecycleConfiguration lifecycleConfiguration;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration, lifecycleConfiguration);
    lifecycleConfiguration.operationMode = static_cast<SilKit_OperationMode>(startConfiguration.operationMode);

    const auto returnCode = SilKit_LifecycleService_Create(&_lifecycleService, participant, &lifecycleConfiguration);
    ThrowOnError(returnCode);
}

void LifecycleService::SetCommunicationReadyHandler(SilKit::Services::Orchestration::CommunicationReadyHandler handler)
{
    auto ownedHandlerPtr = std::make_unique<CommunicationReadyHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
        SILKIT_UNUSED_ARG(lifecycleService);

        const auto handlerPtr = static_cast<CommunicationReadyHandler *>(context);
        (*handlerPtr)();
    };

    const auto returnCode =
        SilKit_LifecycleService_SetCommunicationReadyHandler(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _communicationReadyHandler = std::move(ownedHandlerPtr);
}

void LifecycleService::SetCommunicationReadyHandlerAsync(
    SilKit::Services::Orchestration::CommunicationReadyHandler handler)
{
    auto ownedHandlerPtr = std::make_unique<CommunicationReadyHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
        SILKIT_UNUSED_ARG(lifecycleService);

        const auto handlerPtr = static_cast<CommunicationReadyHandler *>(context);
        (*handlerPtr)();
    };

    const auto returnCode =
        SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _communicationReadyHandlerAsync = std::move(ownedHandlerPtr);
}

void LifecycleService::CompleteCommunicationReadyHandlerAsync()
{
    const auto returnCode = SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(_lifecycleService);
    ThrowOnError(returnCode);
}

void LifecycleService::SetStartingHandler(SilKit::Services::Orchestration::StartingHandler handler)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    auto ownedHandlerPtr = std::make_unique<StartingHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
        SILKIT_UNUSED_ARG(lifecycleService);

        const auto handlerPtr = static_cast<StartingHandler *>(context);
        (*handlerPtr)();
    };

    const auto returnCode =
        SilKit_LifecycleService_SetStartingHandler(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _startingHandler = std::move(ownedHandlerPtr);
}

void LifecycleService::SetStopHandler(SilKit::Services::Orchestration::StopHandler handler)
{
    auto ownedHandlerPtr = std::make_unique<SilKit::Services::Orchestration::StopHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
        SILKIT_UNUSED_ARG(lifecycleService);

        auto handlerPtr = static_cast<SilKit::Services::Orchestration::StopHandler *>(context);
        (*handlerPtr)();
    };

    const auto returnCode = SilKit_LifecycleService_SetStopHandler(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _stopHandler = std::move(ownedHandlerPtr);
}

void LifecycleService::SetShutdownHandler(SilKit::Services::Orchestration::ShutdownHandler handler)
{
    auto ownedHandlerPtr = std::make_unique<ShutdownHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService) {
        SILKIT_UNUSED_ARG(lifecycleService);

        const auto handlerPtr = static_cast<ShutdownHandler *>(context);
        (*handlerPtr)();
    };

    const auto returnCode =
        SilKit_LifecycleService_SetShutdownHandler(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _shutdownHandler = std::move(ownedHandlerPtr);
}

void LifecycleService::SetAbortHandler(SilKit::Services::Orchestration::AbortHandler handler)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    auto ownedHandlerPtr = std::make_unique<AbortHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_LifecycleService *lifecycleService,
                             SilKit_ParticipantState lastParticipantState) {
        SILKIT_UNUSED_ARG(lifecycleService);

        const auto handlerPtr = static_cast<AbortHandler *>(context);
        (*handlerPtr)(static_cast<SilKit::Services::Orchestration::ParticipantState>(lastParticipantState));
    };

    const auto returnCode = SilKit_LifecycleService_SetAbortHandler(_lifecycleService, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _abortHandler = std::move(ownedHandlerPtr);
}

auto LifecycleService::StartLifecycle() -> std::future<ParticipantState>
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

void LifecycleService::ReportError(std::string errorMsg)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_LifecycleService_ReportError(_lifecycleService, errorMsg.c_str());
    ThrowOnError(returnCode);
}

void LifecycleService::Pause(std::string reason)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_LifecycleService_Pause(_lifecycleService, reason.c_str());
    ThrowOnError(returnCode);
}

void LifecycleService::Continue()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_LifecycleService_Continue(_lifecycleService);
    ThrowOnError(returnCode);
}

void LifecycleService::Stop(std::string reason)
{
    const auto returnCode = SilKit_LifecycleService_Stop(_lifecycleService, reason.c_str());
    ThrowOnError(returnCode);
}

auto LifecycleService::State() const -> SilKit::Services::Orchestration::ParticipantState
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_ParticipantState participantState;

    const auto returnCode = SilKit_LifecycleService_State(&participantState, _lifecycleService);
    ThrowOnError(returnCode);

    return static_cast<SilKit::Services::Orchestration::ParticipantState>(participantState);
}

auto LifecycleService::Status() const -> SilKit::Services::Orchestration::ParticipantStatus const &
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    using time_point = decltype(SilKit::Services::Orchestration::ParticipantStatus::enterTime);
    using time_point_duration = typename time_point::duration;
    using duration = std::chrono::nanoseconds;

    SilKit_ParticipantStatus participantStatus;
    SilKit_Struct_Init(SilKit_ParticipantStatus, participantStatus);

    const auto returnCode = SilKit_LifecycleService_Status(&participantStatus, _lifecycleService);
    ThrowOnError(returnCode);

    _lastParticipantStatus.participantName = std::string{participantStatus.participantName};
    _lastParticipantStatus.state =
        static_cast<SilKit::Services::Orchestration::ParticipantState>(participantStatus.participantState);
    _lastParticipantStatus.enterReason = std::string{participantStatus.enterReason};
    _lastParticipantStatus.enterTime =
        time_point{std::chrono::duration_cast<time_point_duration>(duration{participantStatus.enterTime})};
    _lastParticipantStatus.refreshTime =
        time_point{std::chrono::duration_cast<time_point_duration>(duration{participantStatus.refreshTime})};

    return _lastParticipantStatus;
}

auto LifecycleService::CreateTimeSyncService() -> SilKit::Services::Orchestration::ITimeSyncService *
{
    _timeSyncService = std::make_unique<TimeSyncService>(_lifecycleService);

    return _timeSyncService.get();
}

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
