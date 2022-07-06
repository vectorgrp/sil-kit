// Copyright (c) Vector Informatik GmbH. All rights reserved.

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/core/sync/all.hpp"

#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Core::Orchestration;
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
    }

    SilKit_WorkflowConfiguration* workflowConfiguration;

};

void CommunicationReadyCallback(void* /*context*/, SilKit_Participant* /*participant*/) {}
void StopCallback(void* /*context*/, SilKit_Participant* /*participant*/) {}
void ShutdownCallback(void* /*context*/, SilKit_Participant* /*participant*/) {}
void SystemStateHandler(void* /*context*/, SilKit_Participant* /*participant*/, SilKit_SystemState /*state*/) {}
void ParticipantStatusHandler(void* /*context*/, SilKit_Participant* /*participant*/,
                              const char* /*participantName*/, SilKit_ParticipantStatus* /*status*/) {}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_nullpointer_params)
{
    SilKit_ReturnCode returnCode;

    returnCode = SilKit_Participant_SetCommunicationReadyHandler(nullptr, NULL, &CommunicationReadyCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Participant_SetCommunicationReadyHandler((SilKit_Participant*)&mockParticipant, NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_SetStopHandler(nullptr, NULL, &StopCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Participant_SetStopHandler((SilKit_Participant*)&mockParticipant, NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_SetShutdownHandler(nullptr, NULL, &ShutdownCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Participant_SetShutdownHandler((SilKit_Participant*)&mockParticipant, NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // StartLifecycleWithSyncTime
    SilKit_LifecycleConfiguration startConfig;
    returnCode = SilKit_Participant_StartLifecycleWithSyncTime(nullptr, &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_StartLifecycleWithSyncTime((SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // WaitForLifecycleToComplete
    SilKit_ParticipantState outParticipantState;
    returnCode = SilKit_Participant_WaitForLifecycleToComplete(nullptr, &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_WaitForLifecycleToComplete((SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Restart((SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Restart(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Restart(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_RunSimulation(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_StopSimulation(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Pause(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Pause((SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Pause(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_Continue(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ParticipantState participantState;
    returnCode = SilKit_Participant_GetParticipantState(nullptr, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(nullptr, (SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(nullptr, (SilKit_Participant*)&mockParticipant, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(nullptr, nullptr, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(&participantState, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(&participantState, (SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantState(&participantState, nullptr, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_SystemState systemState;
    returnCode = SilKit_Participant_GetSystemState(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetSystemState(&systemState, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetSystemState(nullptr, (SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_HandlerId handlerId;

    // NOTE 2nd parameter 'context' is optional and may be nullptr
    returnCode = SilKit_Participant_AddSystemStateHandler(nullptr, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddSystemStateHandler(nullptr, nullptr, &SystemStateHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddSystemStateHandler((SilKit_Participant*)&mockParticipant, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddSystemStateHandler((SilKit_Participant*)&mockParticipant, nullptr, &SystemStateHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_RemoveSystemStateHandler(nullptr, (SilKit_HandlerId)0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // NOTE 2nd parameter 'context' is optional and may be nullptr
    returnCode = SilKit_Participant_AddParticipantStatusHandler(nullptr, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddParticipantStatusHandler(nullptr, nullptr, &ParticipantStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddParticipantStatusHandler((SilKit_Participant*)&mockParticipant, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_AddParticipantStatusHandler((SilKit_Participant*)&mockParticipant, nullptr, &ParticipantStatusHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_RemoveParticipantStatusHandler(nullptr, (SilKit_HandlerId)0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_SetWorkflowConfiguration(nullptr, workflowConfiguration);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_SetWorkflowConfiguration((SilKit_Participant*)&mockParticipant, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
{
    SilKit_ReturnCode returnCode;
    auto* cParticipant = (SilKit_Participant*)&mockParticipant;

    // required for MockSystemMonitor::ParticipantStatus
    testing::DefaultValue<const SilKit::Core::Orchestration::ParticipantStatus&>::Set(SilKit::Core::Orchestration::ParticipantStatus());

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetCommunicationReadyHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_Participant_SetCommunicationReadyHandler(
        cParticipant,
        nullptr,
        &CommunicationReadyCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetStopHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_Participant_SetStopHandler(
        cParticipant,
        nullptr,
        &StopCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetShutdownHandler(testing::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_SetShutdownHandler(
        cParticipant,
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

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        StartLifecycleWithSyncTime(_, _)
    ).Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_Participant_StartLifecycleWithSyncTime(cParticipant, &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_Participant_WaitForLifecycleToComplete(cParticipant, &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    state = std::promise<ParticipantState> {}; // reset state
    state.set_value(ParticipantState::Shutdown);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        StartLifecycleNoSyncTime(_)
    ).Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_Participant_StartLifecycleNoSyncTime(cParticipant, &startConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_Participant_WaitForLifecycleToComplete(cParticipant, &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_SystemState systemState;
    EXPECT_CALL(mockParticipant.mockSystemMonitor, SystemState()).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_GetSystemState(&systemState, (SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ParticipantState participantState;
    EXPECT_CALL(mockParticipant.mockSystemMonitor, ParticipantStatus(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_GetParticipantState(&participantState, (SilKit_Participant*)&mockParticipant, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Participant_Restart((SilKit_Participant*)&mockParticipant, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Run()).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_RunSimulation((SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Pause(testing::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_Pause((SilKit_Participant*)&mockParticipant, "dummy");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Continue()
    ).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_Continue((SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Stop()).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_StopSimulation((SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, Shutdown(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_Shutdown((SilKit_Participant*)&mockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_HandlerId handlerId;

    EXPECT_CALL(mockParticipant.mockSystemMonitor, AddSystemStateHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_AddSystemStateHandler((SilKit_Participant*)&mockParticipant, nullptr, &SystemStateHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, RemoveSystemStateHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_RemoveSystemStateHandler((SilKit_Participant*)&mockParticipant, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, AddParticipantStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_AddParticipantStatusHandler((SilKit_Participant*)&mockParticipant, nullptr, &ParticipantStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemMonitor, RemoveParticipantStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_RemoveParticipantStatusHandler((SilKit_Participant*)&mockParticipant, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockSystemController, SetWorkflowConfiguration(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_SetWorkflowConfiguration((SilKit_Participant*)&mockParticipant, workflowConfiguration);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} // namespace
