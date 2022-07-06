// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantConfiguration.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/core/sync/all.hpp"
#include "silkit/core/sync/string_utils.hpp"
#include "IParticipantInternal.hpp"

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
SilKit_ReturnCode SilKit_Participant_Create(SilKit_Participant** outParticipant, const char* cParticipantConfigurationString,
                                    const char* cParticipantName, const char* cRegistryUri,
                                    SilKit_Bool /*unused isSynchronized*/)
{
    ASSERT_VALID_OUT_PARAMETER(outParticipant);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantConfigurationString);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantName);
    ASSERT_VALID_POINTER_PARAMETER(cRegistryUri);
    CAPI_ENTER
    {
        auto config = SilKit::Config::ParticipantConfigurationFromString(cParticipantConfigurationString);

        auto participant = SilKit::CreateParticipant(config, cParticipantName, cRegistryUri).release();

        if (participant == nullptr)
        {
            SilKit_error_string =
                "Creating Simulation Participant failed due to unknown error and returned null pointer.";
            return SilKit_ReturnCode_UNSPECIFIEDERROR;
        }

        auto* logger = participant->GetLogger();
        if (logger)
        {
            logger->Info("Creating participant '{}' in domain {}", cParticipantName, cRegistryUri);
        }

        *outParticipant = reinterpret_cast<SilKit_Participant*>(participant);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Destroy(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        if (participant == nullptr)
        {
            SilKit_error_string = "A null pointer argument was passed to the function.";
            return SilKit_ReturnCode_BADPARAMETER;
        }

        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        delete cppParticipant;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_GetLogger(SilKit_Logger** outLogger, SilKit_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto logger = cppParticipant->GetLogger();
        *outLogger = reinterpret_cast<SilKit_Logger*>(logger);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetCommunicationReadyHandler(SilKit_Participant* participant, void* context,
                                                          SilKit_ParticipantCommunicationReadyHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifecycleService = cppParticipant->GetLifecycleService();

        lifecycleService->SetCommunicationReadyHandler([handler, context, participant]() {
            handler(context, participant);
        });

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetStopHandler(SilKit_Participant* participant, void* context,
                                            SilKit_ParticipantStopHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifecycleService = cppParticipant->GetLifecycleService();

        lifecycleService->SetStopHandler([handler, context, participant]() {
            handler(context, participant);
        });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetShutdownHandler(SilKit_Participant* participant, void* context,
                                                SilKit_ParticipantShutdownHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifecycleService = cppParticipant->GetLifecycleService();

        lifecycleService->SetShutdownHandler([handler, context, participant]() {
            handler(context, participant);
        });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// Lifecycle async execution
static std::map<SilKit_Participant*, std::future<SilKit::Core::Orchestration::ParticipantState>> sRunAsyncFuturePerParticipant;

static auto from_c(SilKit_LifecycleConfiguration* csc)
{
    SilKit::Core::Orchestration::LifecycleConfiguration cpp;
    cpp.coordinatedStart = csc->coordinatedStart;
    cpp.coordinatedStop = csc->coordinatedStop;
    return cpp;
}
SilKit_ReturnCode SilKit_Participant_StartLifecycleNoSyncTime(SilKit_Participant* participant,
                                                      SilKit_LifecycleConfiguration* startConfiguration)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(startConfiguration);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifecycleService = cppParticipant->GetLifecycleService();

        sRunAsyncFuturePerParticipant[participant] =
            lifecycleService->StartLifecycleNoSyncTime(from_c(startConfiguration));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_StartLifecycleWithSyncTime(SilKit_Participant* participant,
                                                        SilKit_LifecycleConfiguration* startConfiguration)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(startConfiguration);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifecycleService = cppParticipant->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        sRunAsyncFuturePerParticipant[participant] =
            lifecycleService->StartLifecycleWithSyncTime(timeSyncService, from_c(startConfiguration));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_WaitForLifecycleToComplete(SilKit_Participant* participant,
                                                        SilKit_ParticipantState* outParticipantState)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    CAPI_ENTER
    {
        if (sRunAsyncFuturePerParticipant.find(participant) == sRunAsyncFuturePerParticipant.end())
        {
            SilKit_error_string = "Unknown participant to wait for completion of asynchronous run operation";
            return SilKit_ReturnCode_BADPARAMETER;
        }
        if (!sRunAsyncFuturePerParticipant[participant].valid())
        {
            SilKit_error_string = "Failed to access asynchronous run operation";
            return SilKit_ReturnCode_UNSPECIFIEDERROR;
        }
        auto finalState = sRunAsyncFuturePerParticipant[participant].get();
        *outParticipantState = static_cast<SilKit_ParticipantState>(finalState);
        sRunAsyncFuturePerParticipant.erase(participant);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetPeriod(SilKit_Participant* participant, SilKit_NanosecondsTime period)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipantInternal*>(participant);
        auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
        timeSyncService->SetPeriod(std::chrono::nanoseconds(period));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetSimulationTask(SilKit_Participant* participant, void* context,
                                               SilKit_ParticipantSimulationTaskHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipantInternal*>(participant);
        auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
        timeSyncService->SetSimulationTask(
            [handler, context, participant](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                handler(context, participant, static_cast<SilKit_NanosecondsTime>(now.count()));
            });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetSimulationTaskAsync(SilKit_Participant* participant, void* context,
                                                    SilKit_ParticipantSimulationTaskHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
        timeSyncService->SetSimulationTaskAsync(
            [handler, context, participant](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
                handler(context, participant, static_cast<SilKit_NanosecondsTime>(now.count()));
            });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_CompleteSimulationTask(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
        timeSyncService->CompleteSimulationTask();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Pause(SilKit_Participant* participant, const char* reason)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(reason);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifeCycleService = cppParticipant->GetLifecycleService();
        lifeCycleService->Pause(reason);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Continue(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* lifeCycleService = cppParticipant->GetLifecycleService();
        lifeCycleService->Continue();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Restart(SilKit_Participant* participant, const char* participantName)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemController = cppParticipant->GetSystemController();

        systemController->Restart(participantName);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_RunSimulation(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemController = cppParticipant->GetSystemController();
        systemController->Run();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_StopSimulation(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemController = cppParticipant->GetSystemController();
        systemController->Stop();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_Shutdown(SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipantInternal*>(participant);
        auto* systemController = cppParticipant->GetSystemController();
        systemController->Shutdown(cppParticipant->GetParticipantName());
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_SetWorkflowConfiguration(SilKit_Participant* participant,
                                                      const SilKit_WorkflowConfiguration* workflowConfigration)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(workflowConfigration);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemController = cppParticipant->GetSystemController();
        std::vector<std::string> cppNames;
        assign(cppNames, workflowConfigration->requiredParticipantNames);
        systemController->SetWorkflowConfiguration({cppNames});
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// SystemMonitor related functions
SilKit_ReturnCode SilKit_Participant_GetParticipantState(SilKit_ParticipantState* outParticipantState,
                                                 SilKit_Participant* participant, const char* participantName)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();
        auto& participantStatus = systemMonitor->ParticipantStatus(participantName);
        *outParticipantState = (SilKit_ParticipantState)participantStatus.state;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_GetSystemState(SilKit_SystemState* outParticipantState, SilKit_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outParticipantState);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();
        auto systemState = systemMonitor->SystemState();
        *outParticipantState = (SilKit_SystemState)systemState;
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_AddSystemStateHandler(SilKit_Participant* participant, void* context,
                                                   SilKit_SystemStateHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        auto cppHandlerId = systemMonitor->AddSystemStateHandler(
            [handler, context, participant](SilKit::Core::Orchestration::SystemState systemState) {
                handler(context, participant, (SilKit_SystemState)systemState);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_RemoveSystemStateHandler(SilKit_Participant* participant, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto* cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        systemMonitor->RemoveSystemStateHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_AddParticipantStatusHandler(SilKit_Participant* participant, void* context,
                                                         SilKit_ParticipantStatusHandler_t handler,
                                                         SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        auto cppHandlerId = systemMonitor->AddParticipantStatusHandler(
            [handler, context, participant](SilKit::Core::Orchestration::ParticipantStatus cppStatus) {
                SilKit_ParticipantStatus cStatus;
                cStatus.interfaceId = SilKit_InterfaceIdentifier_ParticipantStatus;
                cStatus.enterReason = cppStatus.enterReason.c_str();
                cStatus.enterTime =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.enterTime.time_since_epoch())
                        .count();
                cStatus.participantName = cppStatus.participantName.c_str();
                cStatus.participantState = (SilKit_ParticipantState)cppStatus.state;
                cStatus.refreshTime =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.refreshTime.time_since_epoch())
                        .count();
                handler(context, participant, cppStatus.participantName.c_str(), &cStatus);
            });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Participant_RemoveParticipantStatusHandler(SilKit_Participant* participant, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto* cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        systemMonitor->RemoveParticipantStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));

        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} //extern "C"
