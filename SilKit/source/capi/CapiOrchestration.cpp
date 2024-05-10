/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/participant/exception.hpp"

#include "participant/ParticipantExtensionsImpl.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <map>
#include <mutex>
#include <cstring>


namespace {

auto CToCpp(const SilKit_LifecycleConfiguration* csc)
{
  SilKit::Services::Orchestration::LifecycleConfiguration cpp{};
  cpp.operationMode = static_cast<decltype(cpp.operationMode)>(csc->operationMode);
  return cpp;
}

void CppToC(
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& cppParticipantConnectionInformation,
    SilKit_ParticipantConnectionInformation& cParticipantConnectionInformation)
{
  SilKit_Struct_Init(SilKit_ParticipantConnectionInformation, cParticipantConnectionInformation);
  cParticipantConnectionInformation.participantName = cppParticipantConnectionInformation.participantName.c_str();
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                 SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outSystemMonitor);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto systemMonitor = cppParticipant->CreateSystemMonitor();
    *outSystemMonitor = reinterpret_cast<SilKit_SystemMonitor*>(systemMonitor);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Create(SilKit_LifecycleService** outLifecycleService,
                                                 SilKit_Participant* participant,
                                                 const SilKit_LifecycleConfiguration* startConfiguration)
try
{
    ASSERT_VALID_OUT_PARAMETER(outLifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(startConfiguration);
    ASSERT_VALID_STRUCT_HEADER(startConfiguration);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto cppLifecycleService = cppParticipant->CreateLifecycleService(CToCpp(startConfiguration));
    *outLifecycleService = reinterpret_cast<SilKit_LifecycleService*>(
        static_cast<SilKit::Services::Orchestration::ILifecycleService*>(cppLifecycleService));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService, SilKit_LifecycleService* lifecycleService)
try
{
    ASSERT_VALID_OUT_PARAMETER(outTimeSyncService);
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);

    auto cppLifecycleService = reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);
    auto cppTimeSyncService = cppLifecycleService->CreateTimeSyncService();

    *outTimeSyncService = reinterpret_cast<SilKit_TimeSyncService*>(cppTimeSyncService);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandler(SilKit_LifecycleService* lifecycleService, void* context,
                                                          SilKit_LifecycleService_CommunicationReadyHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService = reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);

    cppLifecycleService->SetCommunicationReadyHandler([handler, context, lifecycleService]() {
        handler(context, lifecycleService);
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetCommunicationReadyHandlerAsync(SilKit_LifecycleService* lifecycleService, void* context,
                                                          SilKit_LifecycleService_CommunicationReadyHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService = reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);

    cppLifecycleService->SetCommunicationReadyHandlerAsync([handler, context, lifecycleService]() {
        handler(context, lifecycleService);
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_CompleteCommunicationReadyHandlerAsync(SilKit_LifecycleService* lifecycleService)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);

    auto* cppLifecycleService = reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);

    cppLifecycleService->CompleteCommunicationReadyHandlerAsync();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStartingHandler(
    SilKit_LifecycleService* lifecycleService, void* context,
    SilKit_LifecycleService_StartingHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);

    cppLifecycleService->SetStartingHandler([handler, context, lifecycleService]() {
        handler(context, lifecycleService);
    });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* clifecycleService, void* context,
                                            SilKit_LifecycleService_StopHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(clifecycleService);

    cppLifecycleService->SetStopHandler([handler, context, clifecycleService]() {
        handler(context, clifecycleService);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetShutdownHandler(SilKit_LifecycleService* clifecycleService, void* context,
                                                SilKit_LifecycleService_ShutdownHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(clifecycleService);

    cppLifecycleService->SetShutdownHandler([handler, context, clifecycleService]() {
        handler(context, clifecycleService);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_SetAbortHandler(
    SilKit_LifecycleService* lifecycleService, void* context, SilKit_LifecycleService_AbortHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);

    cppLifecycleService->SetAbortHandler(
        [handler, context,
         lifecycleService](SilKit::Services::Orchestration::ParticipantState cppParticipantState) {
            handler(context, lifecycleService, static_cast<SilKit_ParticipantState>(cppParticipantState));
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


namespace {

class LifecycleFutureMap
{
public:
    using ParticipantStateFuture = std::future<SilKit::Services::Orchestration::ParticipantState>;

public:
    void Add(SilKit_LifecycleService * lifecycleService, ParticipantStateFuture future)
    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        _map[lifecycleService] = std::move(future);
    }

    auto Get(SilKit_LifecycleService *lifecycleService) -> ParticipantStateFuture *
    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        const auto it = _map.find(lifecycleService);
        if (it == _map.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    void Remove(SilKit_LifecycleService *lifecycleService)
    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        _map.erase(lifecycleService);
    }

private:
    std::mutex _mutex;
    std::map<SilKit_LifecycleService*, ParticipantStateFuture> _map;
};

auto GetLifecycleFutureMap() -> std::shared_ptr<LifecycleFutureMap>
{
    // XXX: The data stored in the static should be owned and managed by the participant.
    static auto sLifecycleFutureMap = std::make_shared<LifecycleFutureMap>();
    return sLifecycleFutureMap;
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_StartLifecycle(SilKit_LifecycleService* clifecycleService)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(
            clifecycleService);

    GetLifecycleFutureMap()->Add(clifecycleService, cppLifecycleService->StartLifecycle());

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_WaitForLifecycleToComplete(SilKit_LifecycleService* clifecycleService,
                                                        SilKit_ParticipantState* outParticipantState)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);

    auto lifecycleFutureMap = GetLifecycleFutureMap();

    const auto future = lifecycleFutureMap->Get(clifecycleService);
    if (future == nullptr)
    {
        SilKit_error_string = "Unknown participant to wait for completion of asynchronous run operation";
        return SilKit_ReturnCode_BADPARAMETER;
    }
    if (!future->valid())
    {
        SilKit_error_string = "Failed to access asynchronous run operation";
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }
    const auto finalState = future->get();
    *outParticipantState = static_cast<SilKit_ParticipantState>(finalState);
    lifecycleFutureMap->Remove(clifecycleService);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_ReportError(SilKit_LifecycleService* cLifecycleService,
                                                                           const char* reason)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cLifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(reason);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(cLifecycleService);

    cppLifecycleService->ReportError(std::string{reason});

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_State(SilKit_ParticipantState* outParticipantState,
                                                           SilKit_LifecycleService* cLifecycleService)
try
{
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    ASSERT_VALID_POINTER_PARAMETER(cLifecycleService);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(cLifecycleService);

    *outParticipantState = static_cast<SilKit_ParticipantState>(cppLifecycleService->State());

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Status(SilKit_ParticipantStatus* outParticipantStatus,
                                                            SilKit_LifecycleService* cLifecycleService)
try
{
    ASSERT_VALID_OUT_PARAMETER(outParticipantStatus);
    ASSERT_VALID_STRUCT_HEADER(outParticipantStatus);
    ASSERT_VALID_POINTER_PARAMETER(cLifecycleService);

    const auto* lifecycleService =
        reinterpret_cast<const SilKit::Services::Orchestration::ILifecycleService*>(cLifecycleService);
    const auto& participantStatus = lifecycleService->Status();

    outParticipantStatus->enterReason = participantStatus.enterReason.c_str();
    outParticipantStatus->enterTime =
        std::chrono::nanoseconds{participantStatus.enterTime.time_since_epoch()}.count();
    outParticipantStatus->refreshTime =
        std::chrono::nanoseconds{participantStatus.refreshTime.time_since_epoch()}.count();
    outParticipantStatus->participantName = participantStatus.participantName.c_str();
    outParticipantStatus->participantState = (SilKit_ParticipantState)participantStatus.state;

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandler(
    SilKit_TimeSyncService* ctimeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
    SilKit_NanosecondsTime initialStepSize)
try
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);
    timeSyncService->SetSimulationStepHandler(
        [handler, context, ctimeSyncService](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            handler(context, ctimeSyncService, static_cast<SilKit_NanosecondsTime>(now.count()),
                    static_cast<SilKit_NanosecondsTime>(duration.count()));
        },
        std::chrono::nanoseconds(initialStepSize));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
    SilKit_TimeSyncService* ctimeSyncService, void* context, SilKit_TimeSyncService_SimulationStepHandler_t handler,
    SilKit_NanosecondsTime initialStepSize)
try
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);

    timeSyncService->SetSimulationStepHandlerAsync(
        [handler, context, ctimeSyncService](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            handler(context, ctimeSyncService, static_cast<SilKit_NanosecondsTime>(now.count()),
                    static_cast<SilKit_NanosecondsTime>(duration.count()));
        },
        std::chrono::nanoseconds(initialStepSize));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_CompleteSimulationStep(SilKit_TimeSyncService* ctimeSyncService)
try
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);

    auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);
    timeSyncService->CompleteSimulationStep();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_TimeSyncService_Now(SilKit_TimeSyncService* cTimeSyncService,
                                                        SilKit_NanosecondsTime* outNanosecondsTime)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cTimeSyncService);
    ASSERT_VALID_OUT_PARAMETER(outNanosecondsTime);

    auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(cTimeSyncService);
    *outNanosecondsTime = timeSyncService->Now().count();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Pause(SilKit_LifecycleService* clifecycleService, const char* reason)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(reason);

    auto* lifeCycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(clifecycleService);
    lifeCycleService->Pause(reason);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Continue(SilKit_LifecycleService* clifecycleService)
try
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);

    auto* lifeCycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(clifecycleService);
    lifeCycleService->Continue();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LifecycleService_Stop(SilKit_LifecycleService* lifecycleService, const char* reason)
try
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(reason);

    auto* cppLifecycleService =
        reinterpret_cast<SilKit::Services::Orchestration::ILifecycleService*>(lifecycleService);
    cppLifecycleService->Stop(reason);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


// SystemMonitor related functions
SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                            SilKit_SystemMonitor* csystemMonitor,
                                                            const char* participantName)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    ASSERT_VALID_STRUCT_HEADER(outParticipantState);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);
    auto& participantStatus = systemMonitor->ParticipantStatus(participantName);
    SilKit_ParticipantStatus cstatus {};
    SilKit_Struct_Init(SilKit_ParticipantStatus, cstatus);

    cstatus.enterReason = participantStatus.enterReason.c_str();
    cstatus.enterTime = std::chrono::nanoseconds{participantStatus.enterTime.time_since_epoch()}.count();
    cstatus.refreshTime = std::chrono::nanoseconds{participantStatus.refreshTime.time_since_epoch()}.count();
    cstatus.participantName = participantStatus.participantName.c_str();
    cstatus.participantState = (SilKit_ParticipantState)participantStatus.state;

    *outParticipantState = cstatus;
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outParticipantState, SilKit_SystemMonitor* csystemMonitor)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);
    auto systemState = systemMonitor->SystemState();
    *outParticipantState = (SilKit_SystemState)systemState;
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* csystemMonitor, void* context,
                                                   SilKit_SystemStateHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

    auto cppHandlerId = systemMonitor->AddSystemStateHandler(
        [handler, context, csystemMonitor](SilKit::Services::Orchestration::SystemState systemState) {
            handler(context, csystemMonitor, (SilKit_SystemState)systemState);
        });
    *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* csystemMonitor, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

    systemMonitor->RemoveSystemStateHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_AddParticipantStatusHandler(SilKit_SystemMonitor* csystemMonitor, void* context,
                                                         SilKit_ParticipantStatusHandler_t handler,
                                                         SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

    auto cppHandlerId = systemMonitor->AddParticipantStatusHandler(
        [handler, context, csystemMonitor](SilKit::Services::Orchestration::ParticipantStatus cppStatus) {
            SilKit_ParticipantStatus cStatus;
            SilKit_Struct_Init(SilKit_ParticipantStatus, cStatus);

            cStatus.enterReason = cppStatus.enterReason.c_str();
            cStatus.enterTime =
                std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.enterTime.time_since_epoch())
                    .count();
            cStatus.participantName = cppStatus.participantName.c_str();
            cStatus.participantState = (SilKit_ParticipantState)cppStatus.state;
            cStatus.refreshTime =
                std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.refreshTime.time_since_epoch())
                    .count();
            handler(context, csystemMonitor, cppStatus.participantName.c_str(), &cStatus);
        });
    *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* csystemMonitor,
                                                                      SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

    systemMonitor->RemoveParticipantStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantConnectedHandler(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantConnectedHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(systemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppSystemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(systemMonitor);

    cppSystemMonitor->SetParticipantConnectedHandler(
        [handler, context, systemMonitor](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                                              cppParticipantConnectionInformation) {
            SilKit_ParticipantConnectionInformation cParticipantConnectionInformation;
            CppToC(cppParticipantConnectionInformation, cParticipantConnectionInformation);

            handler(context, systemMonitor, &cParticipantConnectionInformation);
        });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_SetParticipantDisconnectedHandler(
    SilKit_SystemMonitor* systemMonitor, void* context, SilKit_SystemMonitor_ParticipantDisconnectedHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(systemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    auto* cppSystemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(systemMonitor);

    cppSystemMonitor->SetParticipantDisconnectedHandler(
        [handler, context, systemMonitor](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                                              cppParticipantConnectionInformation) {
            SilKit_ParticipantConnectionInformation cParticipantConnectionInformation;
            CppToC(cppParticipantConnectionInformation, cParticipantConnectionInformation);

            handler(context, systemMonitor, &cParticipantConnectionInformation);
        });

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_SystemMonitor_IsParticipantConnected(SilKit_SystemMonitor* cSystemMonitor,
                                                                         const char* participantName, SilKit_Bool* out)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSystemMonitor);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    ASSERT_VALID_OUT_PARAMETER(out);

    auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(cSystemMonitor);

    *out = systemMonitor->IsParticipantConnected(std::string{participantName}) ? SilKit_True : SilKit_False;

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_Create(
    SilKit_Experimental_SystemController** outSystemController, SilKit_Participant* participant)
try
{
    ASSERT_VALID_OUT_PARAMETER(outSystemController);
    ASSERT_VALID_POINTER_PARAMETER(participant);

    auto* cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto* cppSystemController = SilKit::Experimental::Participant::CreateSystemControllerImpl(cppParticipant);
    *outSystemController = reinterpret_cast<SilKit_Experimental_SystemController*>(cppSystemController);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_AbortSimulation(
    SilKit_Experimental_SystemController* cSystemController)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSystemController);

    auto* cppSystemController =
        reinterpret_cast<SilKit::Experimental::Services::Orchestration::ISystemController*>(cSystemController);

    cppSystemController->AbortSimulation();

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_SystemController_SetWorkflowConfiguration(
    SilKit_Experimental_SystemController* cSystemController,
    const SilKit_WorkflowConfiguration* cWorkflowConfiguration)
try
{
    ASSERT_VALID_POINTER_PARAMETER(cSystemController);
    ASSERT_VALID_POINTER_PARAMETER(cWorkflowConfiguration);

    auto* cppSystemMonitor =
        reinterpret_cast<SilKit::Experimental::Services::Orchestration::ISystemController*>(cSystemController);

    SilKit::Services::Orchestration::WorkflowConfiguration cppWorkflowConfiguration{};
    for (size_t index = 0; index < cWorkflowConfiguration->requiredParticipantNames->numStrings; ++index)
    {
        cppWorkflowConfiguration.requiredParticipantNames.emplace_back(
            cWorkflowConfiguration->requiredParticipantNames->strings[index]);
    }

    cppSystemMonitor->SetWorkflowConfiguration(cppWorkflowConfiguration);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
