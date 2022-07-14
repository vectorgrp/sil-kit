// Copyright (c) Vector Informatik GmbH. All rights reserved.

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::Orchestration;
using testing::Return;
using testing::ByMove;
using SilKit::Core::Tests::DummyParticipant;

void Create_StringList(SilKit_StringList** outStringList, const char** strings, uint32_t numStrings)
{
    SilKit_StringList* newStrings;
    newStrings = (SilKit_StringList*)malloc(sizeof(SilKit_StringList));
    if (newStrings == nullptr)
    {
        throw std::bad_alloc();
    }
    newStrings->numStrings = numStrings;
    newStrings->strings = (char**)malloc(numStrings * sizeof(char*));
    if (newStrings->strings == nullptr)
    {
        throw std::bad_alloc();
    }
    for (uint32_t i = 0; i < numStrings; i++)
    {
        auto len = strlen(strings[i]) + 1;
        newStrings->strings[i] = (char*)malloc(len);
        if (newStrings->strings[i] != nullptr)
        {
            strcpy((char*)newStrings->strings[i], strings[i]);
        }
    }
    *outStringList = newStrings;
}

class CapiParticipantStateHandlingTest : public testing::Test
{
protected:
    SilKit::Core::Tests::DummyParticipant mockParticipant;

    CapiParticipantStateHandlingTest()
    {
        uint32_t numNames = 2;
        const char* names[2] = {"Participant1", "Participant2"};
        workflowConfiguration = (SilKit_WorkflowConfiguration*)malloc(sizeof(SilKit_WorkflowConfiguration));
        if (workflowConfiguration != NULL)
        {
            Create_StringList(&workflowConfiguration->requiredParticipantNames, names, numNames);
        }

        SilKit_Struct_Init(SilKit_WorkflowConfiguration, *workflowConfiguration);
    }

    SilKit_WorkflowConfiguration* workflowConfiguration;

};

void CommunicationReadyCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void StartingCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void StopCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void ShutdownCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void SystemStateHandler(void* /*context*/, SilKit_SystemMonitor* /*systemMonitor*/, SilKit_SystemState /*state*/) {}
void ParticipantStatusHandler(void* /*context*/, SilKit_SystemMonitor* /*systemMonitor*/,
                              const char* /*participantName*/, SilKit_ParticipantStatus* /*status*/) {}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_nullpointer_params)
{
    SilKit_ReturnCode returnCode;
    SilKit_SystemMonitor* systemMonitor = (SilKit_SystemMonitor*)(mockParticipant.GetSystemMonitor());

    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(nullptr, NULL, &CommunicationReadyCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetStartingHandler(nullptr, NULL, &StartingCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetStartingHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetStopHandler(nullptr, NULL, &StopCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetStopHandler((SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetShutdownHandler(nullptr, NULL, &ShutdownCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetShutdownHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // StartLifecycleWithSyncTime
    SilKit_LifecycleConfiguration startConfig;
    returnCode = SilKit_LifecycleService_StartLifecycleWithSyncTime(nullptr, &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_StartLifecycleWithSyncTime(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // WaitForLifecycleToComplete
    SilKit_ParticipantState outParticipantState;
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(nullptr, &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Restart((SilKit_SystemController*)(mockParticipant.GetSystemController()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Restart(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Restart(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_SystemController_Shutdown((SilKit_SystemController*)(mockParticipant.GetSystemController()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Shutdown(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Run(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_Stop(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_AbortSimulation(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Pause(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_LifecycleService_Pause((SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Pause(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Continue(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ParticipantStatus participantStatus;
    returnCode = SilKit_SystemMonitor_GetParticipantStatus(nullptr, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(nullptr, systemMonitor, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(nullptr, systemMonitor, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(nullptr, nullptr, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(&participantStatus, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(&participantStatus, systemMonitor, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetParticipantStatus(&participantStatus, nullptr, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_SystemState systemState;
    returnCode = SilKit_SystemMonitor_GetSystemState(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetSystemState(&systemState, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_GetSystemState(nullptr, systemMonitor);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_HandlerId handlerId;

    // NOTE 2nd parameter 'context' is optional and may be nullptr
    returnCode = SilKit_SystemMonitor_AddSystemStateHandler(nullptr, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_AddSystemStateHandler(nullptr, nullptr, &SystemStateHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_AddSystemStateHandler(systemMonitor, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_AddSystemStateHandler(systemMonitor, nullptr, &SystemStateHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_RemoveSystemStateHandler(nullptr, (SilKit_HandlerId)0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // NOTE 2nd parameter 'context' is optional and may be nullptr
    returnCode = SilKit_SystemMonitor_AddParticipantStatusHandler(nullptr, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_AddParticipantStatusHandler(nullptr, nullptr, &ParticipantStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_AddParticipantStatusHandler(systemMonitor, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_SystemMonitor_AddParticipantStatusHandler(systemMonitor, nullptr, &ParticipantStatusHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemMonitor_RemoveParticipantStatusHandler(nullptr, (SilKit_HandlerId)0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_SetWorkflowConfiguration(nullptr, workflowConfiguration);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_SystemController_SetWorkflowConfiguration((SilKit_SystemController*)(mockParticipant.GetSystemController()), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
{
    SilKit_ReturnCode returnCode;
    auto* systemMonitor = (SilKit_SystemMonitor*)(mockParticipant.GetSystemMonitor());

    // required for MockSystemMonitor::ParticipantStatus
    SilKit::Services::Orchestration::ParticipantStatus mockParticipantStatus{};
    testing::DefaultValue<const SilKit::Services::Orchestration::ParticipantStatus&>::Set(mockParticipantStatus);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetCommunicationReadyHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()),
        nullptr,
        &CommunicationReadyCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService, SetStartingHandler(testing::_))
        .Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetStartingHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), nullptr, &StartingCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetStopHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetStopHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()),
        nullptr,
        &StopCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetShutdownHandler(testing::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_LifecycleService_SetShutdownHandler(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()),
        nullptr,
        &ShutdownCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    ///////////////////////////////////////////////
    // Lifecycle tests
    ///////////////////////////////////////////////

    using testing::_;
    SilKit_ParticipantState outParticipantState{SilKit_ParticipantState_Invalid};
    std::promise<ParticipantState> state;
    state.set_value(ParticipantState::Shutdown);

    SilKit_LifecycleConfiguration startConfig;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration,startConfig);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        StartLifecycleWithSyncTime(_, _)
    ).Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_LifecycleService_StartLifecycleWithSyncTime(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    state = std::promise<ParticipantState> {}; // reset state
    state.set_value(ParticipantState::Shutdown);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        StartLifecycleNoSyncTime(_)
    ).Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_LifecycleService_StartLifecycleNoSyncTime((SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_SystemState systemState;
    EXPECT_CALL(mockParticipant.mockSystemMonitor, SystemState()).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_GetSystemState(&systemState, systemMonitor);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ParticipantStatus participantStatus;
    SilKit_Struct_Init(SilKit_ParticipantStatus, participantStatus);
    EXPECT_CALL(mockParticipant.mockSystemMonitor, ParticipantStatus(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_GetParticipantStatus(&participantStatus, systemMonitor, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_SystemController_Restart((SilKit_SystemController*)(mockParticipant.GetSystemController()),
                                                 "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Run()).Times(testing::Exactly(1));
    returnCode = SilKit_SystemController_Run((SilKit_SystemController*)(mockParticipant.GetSystemController()));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Pause(testing::_)
    ).Times(testing::Exactly(1));
    returnCode =
        SilKit_LifecycleService_Pause((SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()), "dummy");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Continue()
    ).Times(testing::Exactly(1));
    returnCode = SilKit_LifecycleService_Continue((SilKit_LifecycleService*)(mockParticipant.GetLifecycleService()));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Stop()).Times(testing::Exactly(1));
    returnCode = SilKit_SystemController_Stop((SilKit_SystemController*)(mockParticipant.GetSystemController()));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, AbortSimulation()).Times(testing::Exactly(1));
    returnCode = SilKit_SystemController_AbortSimulation((SilKit_SystemController*)(mockParticipant.GetSystemController()));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Shutdown(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemController_Shutdown((SilKit_SystemController*)(mockParticipant.GetSystemController()), "MockParticipant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_HandlerId handlerId;

    EXPECT_CALL(mockParticipant.mockSystemMonitor, AddSystemStateHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_AddSystemStateHandler(systemMonitor, nullptr, &SystemStateHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, RemoveSystemStateHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_RemoveSystemStateHandler(systemMonitor, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, AddParticipantStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode =
        SilKit_SystemMonitor_AddParticipantStatusHandler(systemMonitor, nullptr, &ParticipantStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, RemoveParticipantStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_RemoveParticipantStatusHandler(systemMonitor, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, SetWorkflowConfiguration(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemController_SetWorkflowConfiguration(
        (SilKit_SystemController*)(mockParticipant.GetSystemController()), workflowConfiguration);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} // namespace
