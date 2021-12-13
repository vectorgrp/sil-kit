/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

//#define IntegrationBusAPI_EXPORT
#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/generic/all.hpp"
#include "ib/sim/generic/string_utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.h"

extern "C" {

IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_Create(ib_SimulationParticipant** outParticipant, const char* cJsonConfig, const char* cParticipantName, const char* cDomainId)
{
  ASSERT_VALID_OUT_PARAMETER(outParticipant);
  ASSERT_VALID_POINTER_PARAMETER(cJsonConfig);
  ASSERT_VALID_POINTER_PARAMETER(cParticipantName);
  ASSERT_VALID_POINTER_PARAMETER(cDomainId);
  CAPI_ENTER
  {
    std::string jsonConfigStr(cJsonConfig);
    std::string participantName(cParticipantName);
    std::string domainIdStr(cDomainId);
    uint32_t domainId = atoi(domainIdStr.c_str());

    auto ibConfig = ib::cfg::Config::FromJsonString(jsonConfigStr);

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId).release();
        
    if (comAdapter == nullptr)
    {
      ib_error_string = "Creating Simulation Participant failed due to unknown error and returned null pointer.";
      return ib_ReturnCode_UNSPECIFIEDERROR;
    }

    *outParticipant = reinterpret_cast<ib_SimulationParticipant*>(comAdapter);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_Destroy(ib_SimulationParticipant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    if (participant == nullptr)
    {
        ib_error_string = "A null pointer argument was passed to the function.";
        return ib_ReturnCode_BADPARAMETER;
    }

    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    delete comAdapter;
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

IntegrationBusAPI ib_ReturnCode ib_SimulationParticipant_GetLogger(ib_Logger** outLogger, ib_SimulationParticipant* participant)
{
    ASSERT_VALID_OUT_PARAMETER(outLogger);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    CAPI_ENTER
    {
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        auto logger = comAdapter->GetLogger();
        *outLogger = reinterpret_cast<ib_Logger*>(logger);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_SetInitHandler(ib_SimulationParticipant* participant, void* context, ib_ParticipantInitHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();

    participantController->SetInitHandler(
      [handler, context, participant](ib::mw::sync::ParticipantCommand initCmd) {
          ib_ParticipantCommand command;
          command.kind = (ib_ParticipantCommand_Kind)initCmd.kind;
          handler(context, participant, &command);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_SetStopHandler(ib_SimulationParticipant* participant, void* context, ib_ParticipantStopHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();

    participantController->SetStopHandler(
      [handler, context, participant]() {
          handler(context, participant);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_SetShutdownHandler(ib_SimulationParticipant* participant, void* context, ib_ParticipantShutdownHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();

    participantController->SetShutdownHandler(
      [handler, context, participant]() {
          handler(context, participant);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_Run(ib_SimulationParticipant* participant,
                                           ib_ParticipantState*      outParticipantState)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_OUT_PARAMETER(outParticipantState);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();
    auto finalState = participantController->Run();
    *outParticipantState = static_cast<ib_ParticipantState>(finalState);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

static std::map<ib_SimulationParticipant*, std::future<ib::mw::sync::ParticipantState>> sRunAsyncFuturePerParticipant;
ib_ReturnCode ib_SimulationParticipant_RunAsync(ib_SimulationParticipant* participant)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    if (sRunAsyncFuturePerParticipant.find(participant) != sRunAsyncFuturePerParticipant.end())
    {
      ib_error_string = "ib_SimulationParticipant_RunAsync has already been called for participant";
      return ib_ReturnCode_BADPARAMETER;
    }
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();
    sRunAsyncFuturePerParticipant[participant] = participantController->RunAsync();
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_WaitForRunAsyncToComplete(ib_SimulationParticipant* participant,
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

ib_ReturnCode ib_SimulationParticipant_SetPeriod(ib_SimulationParticipant* participant, ib_NanosecondsTime period) 
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();
    participantController->SetPeriod(std::chrono::nanoseconds(period)); 
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_SimulationParticipant_SetSimulationTask(ib_SimulationParticipant* participant, void* context, ib_ParticipantSimulationTaskHandler_t handler)
{
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_HANDLER_PARAMETER(handler);
  CAPI_ENTER
  {
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto* participantController = comAdapter->GetParticipantController();
    participantController->SetSimulationTask(
      [handler, context, participant](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
          handler(context, participant, static_cast<ib_NanosecondsTime>(now.count()));
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

}
