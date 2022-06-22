/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

//#define IntegrationBusAPI_EXPORT
#include "ParticipantConfiguration.hpp"

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "IParticipantInternal.hpp"

#include "CapiImpl.h"
#include "TypeConversion.hpp"

#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

extern "C" {

ib_ReturnCode ib_Participant_Create(ib_Participant** outParticipant, const char* cParticipantConfigurationString,
                                                  const char* cParticipantName, const char* cDomainId,
                                                  ib_Bool /*unused isSynchronized*/)
{
    ASSERT_VALID_OUT_PARAMETER(outParticipant);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantConfigurationString);
    ASSERT_VALID_POINTER_PARAMETER(cParticipantName);
    ASSERT_VALID_POINTER_PARAMETER(cDomainId);
    CAPI_ENTER
    {
        std::string participantConfigurationStr(cParticipantConfigurationString);
        std::string participantName(cParticipantName);
        std::string domainIdStr(cDomainId);
        uint32_t domainId = atoi(domainIdStr.c_str());

        auto ibConfig = ib::cfg::ParticipantConfigurationFromString(participantConfigurationStr);

        auto participant =
            ib::CreateParticipant(ibConfig, participantName, domainId).release();

        if (participant == nullptr)
        {
            ib_error_string =
                "Creating Simulation Participant failed due to unknown error and returned null pointer.";
            return ib_ReturnCode_UNSPECIFIEDERROR;
        }

        auto* logger = participant->GetLogger();
        if(logger)
        {
            logger->Info("Creating participant '{}' in domain {}",
                participantName, domainId);
        }

        *outParticipant = reinterpret_cast<ib_Participant*>(participant);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Participant_Destroy(ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    if (participant == nullptr)
    {
        ib_error_string = "A null pointer argument was passed to the function.";
        return ib_ReturnCode_BADPARAMETER;
    }

    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    delete cppParticipant;
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_GetLogger(ib_Logger** outLogger, ib_Participant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto logger = cppParticipant->GetLogger();
        *outLogger = reinterpret_cast<ib_Logger*>(logger);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetInitHandler(ib_Participant* participant, void* context, ib_ParticipantInitHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* lifecycleService = cppParticipant->GetLifecycleService();

    lifecycleService->SetCommunicationReadyHandler(
      [handler, context, participant]() {
          handler(context, participant);
      });

    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetStopHandler(ib_Participant* participant, void* context, ib_ParticipantStopHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* lifecycleService = cppParticipant->GetLifecycleService();

    lifecycleService->SetStopHandler(
      [handler, context, participant]() {
          handler(context, participant);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetShutdownHandler(ib_Participant* participant, void* context, ib_ParticipantShutdownHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* lifecycleService = cppParticipant->GetLifecycleService();

    lifecycleService->SetShutdownHandler(
      [handler, context, participant]() {
          handler(context, participant);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

// Lifecycle async execution
static std::map<ib_Participant*, std::future<ib::mw::sync::ParticipantState>> sRunAsyncFuturePerParticipant;

ib_ReturnCode ib_Participant_ExecuteLifecycleNoSyncTime(
    ib_Participant* participant,
    ib_Bool hasCoordinatedSimulationStart,
    ib_Bool hasCoordinatedSimulationStop,
    ib_Bool isRequiredParticipant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_BOOL_PARAMETER(hasCoordinatedSimulationStart);
  ASSERT_VALID_BOOL_PARAMETER(hasCoordinatedSimulationStop);
  ASSERT_VALID_BOOL_PARAMETER(isRequiredParticipant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* lifecycleService = cppParticipant->GetLifecycleService();

    sRunAsyncFuturePerParticipant[participant] =
        lifecycleService->ExecuteLifecycleNoSyncTime(
            hasCoordinatedSimulationStart == ib_True,
            hasCoordinatedSimulationStop == ib_True,
            isRequiredParticipant == ib_True);


    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


ib_ReturnCode ib_Participant_ExecuteLifecycleWithSyncTime(
    ib_Participant* participant,
    ib_Bool hasCoordinatedSimulationStart,
    ib_Bool hasCoordinatedSimulationStop,
    ib_Bool isRequiredParticipant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_BOOL_PARAMETER(hasCoordinatedSimulationStart);
  ASSERT_VALID_BOOL_PARAMETER(hasCoordinatedSimulationStop);
  ASSERT_VALID_BOOL_PARAMETER(isRequiredParticipant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* lifecycleService = cppParticipant->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();

    sRunAsyncFuturePerParticipant[participant] =
        lifecycleService->ExecuteLifecycleWithSyncTime(
            timeSyncService,
            hasCoordinatedSimulationStart == ib_True,
            hasCoordinatedSimulationStop == ib_True,
            isRequiredParticipant == ib_True);

    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_WaitForLifecycleToComplete(ib_Participant* participant,
                                                                 ib_ParticipantState*      outParticipantState)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_OUT_PARAMETER(outParticipantState);
  CAPI_ENTER
  {
    if (sRunAsyncFuturePerParticipant.find(participant) == sRunAsyncFuturePerParticipant.end())
    {
      ib_error_string = "Unknown participant to wait for completion of asynchronous run operation";
      return ib_ReturnCode_BADPARAMETER;
    }
    if (!sRunAsyncFuturePerParticipant[participant].valid())
    {
      ib_error_string = "Failed to access asynchronous run operation";
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }
    auto finalState = sRunAsyncFuturePerParticipant[participant].get();
    *outParticipantState = static_cast<ib_ParticipantState>(finalState);
    sRunAsyncFuturePerParticipant.erase(participant); 
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetPeriod(ib_Participant* participant, ib_NanosecondsTime period) 
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipantInternal*>(participant);
    auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
    timeSyncService->SetPeriod(std::chrono::nanoseconds(period)); 
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetSimulationTask(ib_Participant* participant, void* context, ib_ParticipantSimulationTaskHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipantInternal*>(participant);
    auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
    timeSyncService->SetSimulationTask(
      [handler, context, participant](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
          handler(context, participant, static_cast<ib_NanosecondsTime>(now.count()));
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetSimulationTaskAsync(ib_Participant* participant, void* context, ib_ParticipantSimulationTaskHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
    timeSyncService->SetSimulationTaskAsync(
      [handler, context, participant](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
          handler(context, participant, static_cast<ib_NanosecondsTime>(now.count()));
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_CompleteSimulationTask(ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* timeSyncService = cppParticipant->GetLifecycleService()->GetTimeSyncService();
    timeSyncService->CompleteSimulationTask();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_Pause(ib_Participant* participant, const char* reason)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(reason);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto* lifeCycleService = cppParticipant->GetLifecycleService();
        lifeCycleService->Pause(reason);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Participant_Continue(ib_Participant* participant)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto* lifeCycleService = cppParticipant->GetLifecycleService();
        lifeCycleService->Continue();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// SystemController related functions
ib_ReturnCode ib_Participant_Initialize(ib_Participant* participant, const char* participantName)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(participantName);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    std::string name{ participantName };
    auto* systemController = cppParticipant->GetSystemController();

    systemController->Initialize(name);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_Restart(ib_Participant* participant, const char* participantName)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(participantName);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemController = cppParticipant->GetSystemController();

    systemController->Restart(participantName);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_RunSimulation(ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemController = cppParticipant->GetSystemController();
    systemController->Run();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_StopSimulation(ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemController = cppParticipant->GetSystemController();
    systemController->Stop();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


ib_ReturnCode ib_Participant_Shutdown(ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemController = cppParticipant->GetSystemController();
    systemController->Shutdown();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_SetRequiredParticipants(ib_Participant* participant,
                                                     const ib_StringList* requiredParticipantNames)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(requiredParticipantNames);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto* systemController = cppParticipant->GetSystemController();
        std::vector<std::string> cppNames;
        assign(cppNames, requiredParticipantNames);
        systemController->SetRequiredParticipants(cppNames);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

// SystemMonitor related functions
ib_ReturnCode ib_Participant_GetParticipantState(ib_ParticipantState* outParticipantState, ib_Participant* participant, const char* participantName)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_OUT_PARAMETER(outParticipantState);
  ASSERT_VALID_POINTER_PARAMETER(participantName);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemMonitor = cppParticipant->GetSystemMonitor();
    auto& participantStatus = systemMonitor->ParticipantStatus(participantName);
    *outParticipantState = (ib_ParticipantState)participantStatus.state;
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_GetSystemState(ib_SystemState* outParticipantState, ib_Participant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_OUT_PARAMETER(outParticipantState);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemMonitor = cppParticipant->GetSystemMonitor();
    auto systemState = systemMonitor->SystemState();
    *outParticipantState = (ib_SystemState)systemState;
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_AddSystemStateHandler(ib_Participant* participant, void* context, ib_SystemStateHandler_t handler, ib_HandlerId* outHandlerId)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  ASSERT_VALID_OUT_PARAMETER(outHandlerId);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemMonitor = cppParticipant->GetSystemMonitor();

    auto cppHandlerId = systemMonitor->AddSystemStateHandler(
      [handler, context, participant](ib::mw::sync::SystemState systemState) {
          handler(context, participant, (ib_SystemState)systemState);
      });
    *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_RemoveSystemStateHandler(ib_Participant* participant, ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto* cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        systemMonitor->RemoveSystemStateHandler(static_cast<ib::util::HandlerId>(handlerId));

        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Participant_AddParticipantStatusHandler(ib_Participant* participant, void* context, ib_ParticipantStatusHandler_t handler, ib_HandlerId* outHandlerId)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  ASSERT_VALID_OUT_PARAMETER(outHandlerId);
  CAPI_ENTER
  {
    auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
    auto* systemMonitor = cppParticipant->GetSystemMonitor();

    auto cppHandlerId = systemMonitor->AddParticipantStatusHandler(
      [handler, context, participant](ib::mw::sync::ParticipantStatus cppStatus) {
            ib_ParticipantStatus cStatus;
            cStatus.interfaceId = ib_InterfaceIdentifier_ParticipantStatus;
            cStatus.enterReason = cppStatus.enterReason.c_str();
            cStatus.enterTime = std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.enterTime.time_since_epoch()).count();
            cStatus.participantName = cppStatus.participantName.c_str();
            cStatus.participantState = (ib_ParticipantState)cppStatus.state;
            cStatus.refreshTime = std::chrono::duration_cast<std::chrono::nanoseconds>(cppStatus.refreshTime.time_since_epoch()).count();
          handler(context, participant, cppStatus.participantName.c_str(), &cStatus);
      });
    *outHandlerId = static_cast<ib_HandlerId>(cppHandlerId);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_Participant_RemoveParticipantStatusHandler(ib_Participant* participant, ib_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto* cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto* systemMonitor = cppParticipant->GetSystemMonitor();

        systemMonitor->RemoveParticipantStatusHandler(static_cast<ib::util::HandlerId>(handlerId));

        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
