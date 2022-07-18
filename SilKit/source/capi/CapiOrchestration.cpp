// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantConfiguration.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "IParticipantInternal.hpp"
#include "LifecycleService.hpp"
#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

extern "C"
{

SilKit_ReturnCode SilKit_SystemMonitor_Create(SilKit_SystemMonitor** outSystemMonitor,
                                                 SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outSystemMonitor);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto systemMonitor = cppParticipant->CreateSystemMonitor();
        *outSystemMonitor = reinterpret_cast<SilKit_SystemMonitor*>(systemMonitor);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_Create(SilKit_SystemController** outSystemController,
                                                 SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outSystemController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto cppLifecycleService = cppParticipant->CreateSystemController();
        *outSystemController = reinterpret_cast<SilKit_SystemController*>(cppLifecycleService);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleServiceNoTimeSync_Create(SilKit_LifecycleService** outLifecycleService,
                                                 SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto cppLifecycleService = cppParticipant->CreateLifecycleServiceNoTimeSync();
        *outLifecycleService = reinterpret_cast<SilKit_LifecycleService*>(
            static_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(cppLifecycleService));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleServiceWithTimeSync_Create(SilKit_LifecycleService** outLifecycleService,
                                                           SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto cppLifecycleService = cppParticipant->CreateLifecycleServiceWithTimeSync();
        *outLifecycleService = reinterpret_cast<SilKit_LifecycleService*>(
            static_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(cppLifecycleService));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_TimeSyncService_Create(SilKit_TimeSyncService** outTimeSyncService, SilKit_LifecycleService* lifecycleService)
{
    ASSERT_VALID_OUT_PARAMETER(outTimeSyncService);
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    CAPI_ENTER
    {
        auto cppLifecycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(lifecycleService);
        auto timeSyncService = cppLifecycleService->GetTimeSyncService();
        *outTimeSyncService = reinterpret_cast<SilKit_TimeSyncService*>(timeSyncService);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_SetCommunicationReadyHandler(SilKit_LifecycleService* lifecycleService, void* context,
                                                          SilKit_LifecycleService_CommunicationReadyHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* cppLifecycleService = reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(lifecycleService);
        
        cppLifecycleService->SetCommunicationReadyHandler([handler, context, lifecycleService]() {
            handler(context, lifecycleService);
        });

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_SetStartingHandler(
    SilKit_LifecycleService* lifecycleService, void* context,
    SilKit_LifecycleService_StartingHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(lifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* cppLifecycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(lifecycleService);

        cppLifecycleService->SetStartingHandler([handler, context, lifecycleService]() {
            handler(context, lifecycleService);
        });

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_SetStopHandler(SilKit_LifecycleService* clifecycleService, void* context,
                                            SilKit_LifecycleService_StopHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* cppLifecycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(clifecycleService);

        cppLifecycleService->SetStopHandler([handler, context, clifecycleService]() {
            handler(context, clifecycleService);
        });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_SetShutdownHandler(SilKit_LifecycleService* clifecycleService, void* context,
                                                SilKit_LifecycleService_ShutdownHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* cppLifecycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(clifecycleService);

        cppLifecycleService->SetShutdownHandler([handler, context, clifecycleService]() {
            handler(context, clifecycleService);
        });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// Lifecycle async execution
static std::map<SilKit_LifecycleService*, std::future<SilKit::Services::Orchestration::ParticipantState>> sRunAsyncFuturePerParticipant;

static auto from_c(SilKit_LifecycleConfiguration* csc)
{
    SilKit::Services::Orchestration::LifecycleConfiguration cpp;
    cpp.coordinatedStart = csc->coordinatedStart;
    cpp.coordinatedStop = csc->coordinatedStop;
    return cpp;
}
SilKit_ReturnCode SilKit_LifecycleService_StartLifecycle(SilKit_LifecycleService* clifecycleService,
                                                      SilKit_LifecycleConfiguration* startConfiguration)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(startConfiguration);
    ASSERT_VALID_STRUCT_HEADER(startConfiguration);
    CAPI_ENTER
    {
        auto* cppLifecycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(
                clifecycleService);

        sRunAsyncFuturePerParticipant[clifecycleService] =
            cppLifecycleService->StartLifecycle(from_c(startConfiguration));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_WaitForLifecycleToComplete(SilKit_LifecycleService* clifecycleService,
                                                        SilKit_ParticipantState* outParticipantState)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    CAPI_ENTER
    {
        if (sRunAsyncFuturePerParticipant.find(clifecycleService) == sRunAsyncFuturePerParticipant.end())
        {
            SilKit_error_string = "Unknown participant to wait for completion of asynchronous run operation";
            return SilKit_ReturnCode_BADPARAMETER;
        }
        if (!sRunAsyncFuturePerParticipant[clifecycleService].valid())
        {
            SilKit_error_string = "Failed to access asynchronous run operation";
            return SilKit_ReturnCode_UNSPECIFIEDERROR;
        }
        auto finalState = sRunAsyncFuturePerParticipant[clifecycleService].get();
        *outParticipantState = static_cast<SilKit_ParticipantState>(finalState);
        sRunAsyncFuturePerParticipant.erase(clifecycleService);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationStepHandler(
    SilKit_TimeSyncService* ctimeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler,
    SilKit_NanosecondsTime initialStepSize)
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);
        timeSyncService->SetSimulationStepHandler(
            [handler, context, ctimeSyncService](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                handler(context, ctimeSyncService, static_cast<SilKit_NanosecondsTime>(now.count()));
            },
            std::chrono::nanoseconds(initialStepSize));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_TimeSyncService_SetSimulationStepHandlerAsync(
    SilKit_TimeSyncService* ctimeSyncService, void* context, SilKit_TimeSyncService_SimulationTaskHandler_t handler,
    SilKit_NanosecondsTime initialStepSize)
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);

        timeSyncService->SetSimulationStepHandlerAsync(
            [handler, context, ctimeSyncService](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                handler(context, ctimeSyncService, static_cast<SilKit_NanosecondsTime>(now.count()));
            },
            std::chrono::nanoseconds(initialStepSize));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_TimeSyncService_CompleteSimulationStep(SilKit_TimeSyncService* ctimeSyncService)
{
    ASSERT_VALID_POINTER_PARAMETER(ctimeSyncService);
    CAPI_ENTER
    {
        auto* timeSyncService = reinterpret_cast<SilKit::Services::Orchestration::ITimeSyncService*>(ctimeSyncService);
        timeSyncService->CompleteSimulationTask();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_Pause(SilKit_LifecycleService* clifecycleService, const char* reason)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    ASSERT_VALID_POINTER_PARAMETER(reason);
    CAPI_ENTER
    {
        auto* lifeCycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(clifecycleService);
        lifeCycleService->Pause(reason);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LifecycleService_Continue(SilKit_LifecycleService* clifecycleService)
{
    ASSERT_VALID_POINTER_PARAMETER(clifecycleService);
    CAPI_ENTER
    {
        auto* lifeCycleService =
            reinterpret_cast<SilKit::Services::Orchestration::ILifecycleServiceInternal*>(clifecycleService);
        lifeCycleService->Continue();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_Restart(SilKit_SystemController* csystemController, const char* participantName)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    CAPI_ENTER
    {
        auto* systemController = reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);

        systemController->Restart(participantName);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_Run(SilKit_SystemController* csystemController)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    CAPI_ENTER
    {
        auto* systemController = reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);
        systemController->Run();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_Stop(SilKit_SystemController* csystemController)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    CAPI_ENTER
    {
        auto* systemController = reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);
        systemController->Stop();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_Shutdown(SilKit_SystemController* csystemController, const char* participantName)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    CAPI_ENTER
    {
        auto* systemController =
            reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);
        systemController->Shutdown(participantName);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_AbortSimulation(SilKit_SystemController* csystemController)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    CAPI_ENTER
    {
        auto* systemController =
            reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);
        systemController->AbortSimulation();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemController_SetWorkflowConfiguration(
    SilKit_SystemController* csystemController, const SilKit_WorkflowConfiguration* workflowConfigration)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemController);
    ASSERT_VALID_POINTER_PARAMETER(workflowConfigration);
    ASSERT_VALID_STRUCT_HEADER(workflowConfigration);
    CAPI_ENTER
    {
        auto* systemController = reinterpret_cast<SilKit::Services::Orchestration::ISystemController*>(csystemController);
        std::vector<std::string> cppNames;
        assign(cppNames, workflowConfigration->requiredParticipantNames);
        systemController->SetWorkflowConfiguration({cppNames});
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// SystemMonitor related functions
SilKit_ReturnCode SilKit_SystemMonitor_GetParticipantStatus(SilKit_ParticipantStatus* outParticipantState,
                                                            SilKit_SystemMonitor* csystemMonitor,
                                                            const char* participantName)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    ASSERT_VALID_STRUCT_HEADER(outParticipantState);
    CAPI_ENTER
    {
        auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);
        auto& participantStatus = systemMonitor->ParticipantStatus(participantName);
        SilKit_ParticipantStatus cstatus {};
        SilKit_Struct_Init(SilKit_ParticipantStatus, cstatus);

        cstatus.enterReason = participantStatus.enterReason.c_str();
        cstatus.enterTime = participantStatus.enterTime.time_since_epoch().count();
        cstatus.participantName = participantStatus.participantName.c_str();
        cstatus.participantState = (SilKit_ParticipantState)participantStatus.state;

        *outParticipantState = cstatus;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemMonitor_GetSystemState(SilKit_SystemState* outParticipantState, SilKit_SystemMonitor* csystemMonitor)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    CAPI_ENTER
    {
        auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);
        auto systemState = systemMonitor->SystemState();
        *outParticipantState = (SilKit_SystemState)systemState;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemMonitor_AddSystemStateHandler(SilKit_SystemMonitor* csystemMonitor, void* context,
                                                   SilKit_SystemStateHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

        auto cppHandlerId = systemMonitor->AddSystemStateHandler(
            [handler, context, csystemMonitor](SilKit::Services::Orchestration::SystemState systemState) {
                handler(context, csystemMonitor, (SilKit_SystemState)systemState);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemMonitor_RemoveSystemStateHandler(SilKit_SystemMonitor* csystemMonitor, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    CAPI_ENTER
    {
        auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

        systemMonitor->RemoveSystemStateHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemMonitor_AddParticipantStatusHandler(SilKit_SystemMonitor* csystemMonitor, void* context,
                                                         SilKit_ParticipantStatusHandler_t handler,
                                                         SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
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
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_SystemMonitor_RemoveParticipantStatusHandler(SilKit_SystemMonitor* csystemMonitor,
                                                                      SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(csystemMonitor);
    CAPI_ENTER
    {
        auto* systemMonitor = reinterpret_cast<SilKit::Services::Orchestration::ISystemMonitor*>(csystemMonitor);

        systemMonitor->RemoveParticipantStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} //extern "C"
