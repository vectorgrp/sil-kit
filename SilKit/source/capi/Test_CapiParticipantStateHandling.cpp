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

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include "MockParticipant.hpp"

namespace {
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

    std::unique_ptr<SilKit_WorkflowConfiguration> workflowConfiguration;

    CapiParticipantStateHandlingTest()
    {
        uint32_t numNames = 2;
        const char* names[2] = {"Participant1", "Participant2"};

        workflowConfiguration = std::make_unique<SilKit_WorkflowConfiguration>();
        [this] {
            ASSERT_NE(workflowConfiguration, nullptr);
        }();

        SilKit_Struct_Init(SilKit_WorkflowConfiguration, *workflowConfiguration);
        Create_StringList(&workflowConfiguration->requiredParticipantNames, names, numNames);
    }

    ~CapiParticipantStateHandlingTest()
    {
        for (uint32_t index = 0; index != workflowConfiguration->requiredParticipantNames->numStrings; ++index)
        {
            free(workflowConfiguration->requiredParticipantNames->strings[index]);
        }
        free(workflowConfiguration->requiredParticipantNames->strings);
        free(workflowConfiguration->requiredParticipantNames);
    }
};

void SilKitCALL CommunicationReadyCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void SilKitCALL StartingCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void SilKitCALL StopCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void SilKitCALL ShutdownCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/) {}
void SilKitCALL AbortCallback(void* /*context*/, SilKit_LifecycleService* /*lifecycleService*/,
                   SilKit_ParticipantState /*lastParticipantState*/) {}
void SilKitCALL SystemStateHandler(void* /*context*/, SilKit_SystemMonitor* /*systemMonitor*/, SilKit_SystemState /*state*/) {}
void SilKitCALL ParticipantStatusHandler(void* /*context*/, SilKit_SystemMonitor* /*systemMonitor*/,
                              const char* /*participantName*/, SilKit_ParticipantStatus* /*status*/) {}

/*
* check whether the api rejects bad parameters
*/
TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_nullpointer_params)
{
    SilKit_ReturnCode returnCode;
    SilKit_SystemMonitor* systemMonitor = (SilKit_SystemMonitor*)(mockParticipant.GetSystemMonitor());
    auto* lifecycleService = (SilKit_LifecycleService*)(mockParticipant.GetLifecycleService());

    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(nullptr, NULL, &CommunicationReadyCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(
        (SilKit_LifecycleService*)(lifecycleService), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetStartingHandler(nullptr, NULL, &StartingCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetStartingHandler(
        (SilKit_LifecycleService*)(lifecycleService), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetStopHandler(nullptr, NULL, &StopCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetStopHandler((SilKit_LifecycleService*)(lifecycleService), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetShutdownHandler(nullptr, NULL, &ShutdownCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetShutdownHandler(
        (SilKit_LifecycleService*)(lifecycleService), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_SetAbortHandler(nullptr, NULL, &AbortCallback);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LifecycleService_SetAbortHandler(
        (SilKit_LifecycleService*)(lifecycleService), NULL, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // StartLifecycle
    returnCode = SilKit_LifecycleService_StartLifecycle(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    // WaitForLifecycleToComplete
    SilKit_ParticipantState outParticipantState;
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(nullptr, &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(lifecycleService), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Pause(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_LifecycleService_Pause((SilKit_LifecycleService*)(lifecycleService), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Pause(nullptr, "test");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Stop(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_LifecycleService_Stop((SilKit_LifecycleService*)(lifecycleService), nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LifecycleService_Stop(nullptr, "test");
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

}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_no_time_sync_function_mapping)
{
    SilKit_ReturnCode returnCode;
    auto* systemMonitor = (SilKit_SystemMonitor*)(mockParticipant.GetSystemMonitor());
    auto* lifecycleService =
        (SilKit_LifecycleService*)(static_cast<SilKit::Services::Orchestration::ILifecycleService*>(
            mockParticipant.CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Invalid})));

    EXPECT_CALL(mockParticipant.mockLifecycleService, SetStartingHandler(testing::_)).Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetStartingHandler((SilKit_LifecycleService*)(lifecycleService), nullptr,
                                                            &StartingCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    ///////////////////////////////////////////////
    // Lifecycle tests
    ///////////////////////////////////////////////

    using testing::_;
    SilKit_ParticipantState outParticipantState{SilKit_ParticipantState_Invalid};
    std::promise<SilKit::Services::Orchestration::ParticipantState> state;
    state.set_value(SilKit::Services::Orchestration::ParticipantState::Shutdown);

    SilKit_LifecycleConfiguration startConfig;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration, startConfig);

    EXPECT_CALL(mockParticipant.mockLifecycleService, StartLifecycle())
        .Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));

    returnCode =
        SilKit_LifecycleService_StartLifecycle((SilKit_LifecycleService*)(lifecycleService));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete((SilKit_LifecycleService*)(lifecycleService),
                                                                    &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_SystemState systemState;
    EXPECT_CALL(mockParticipant.mockSystemMonitor, SystemState()).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_GetSystemState(&systemState, systemMonitor);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit::Services::Orchestration::ParticipantStatus cppParticipantStatus{};
    ON_CALL(mockParticipant.mockSystemMonitor, ParticipantStatus(testing::_))
            .WillByDefault(testing::ReturnRef(cppParticipantStatus));

    SilKit_ParticipantStatus participantStatus;
    SilKit_Struct_Init(SilKit_ParticipantStatus, participantStatus);
    EXPECT_CALL(mockParticipant.mockSystemMonitor, ParticipantStatus(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_SystemMonitor_GetParticipantStatus(&participantStatus, systemMonitor, "participant");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
{
    SilKit_ReturnCode returnCode;
    auto* systemMonitor = (SilKit_SystemMonitor*)(mockParticipant.GetSystemMonitor());
    auto* lifecycleService = (SilKit_LifecycleService*)(mockParticipant.CreateLifecycleService(
        {SilKit::Services::Orchestration::OperationMode::Coordinated}));

    // required for MockSystemMonitor::ParticipantStatus
    SilKit::Services::Orchestration::ParticipantStatus mockParticipantStatus{};
    testing::DefaultValue<const SilKit::Services::Orchestration::ParticipantStatus&>::Set(mockParticipantStatus);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetCommunicationReadyHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetCommunicationReadyHandler(
        (SilKit_LifecycleService*)(lifecycleService),
        nullptr,
        &CommunicationReadyCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetStopHandler(testing::_)
    ).Times(testing::Exactly(1));

    returnCode = SilKit_LifecycleService_SetStopHandler(
        (SilKit_LifecycleService*)(lifecycleService),
        nullptr,
        &StopCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        SetShutdownHandler(testing::_)
    ).Times(testing::Exactly(1));
    returnCode = SilKit_LifecycleService_SetShutdownHandler(
        (SilKit_LifecycleService*)(lifecycleService),
        nullptr,
        &ShutdownCallback);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    ///////////////////////////////////////////////
    // Lifecycle tests
    ///////////////////////////////////////////////

    using testing::_;
    SilKit_ParticipantState outParticipantState{SilKit_ParticipantState_Invalid};
    std::promise<SilKit::Services::Orchestration::ParticipantState> state;
    state.set_value(SilKit::Services::Orchestration::ParticipantState::Shutdown);

    SilKit_LifecycleConfiguration startConfig;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration,startConfig);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        StartLifecycle()
    ).Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_LifecycleService_StartLifecycle((SilKit_LifecycleService*)(lifecycleService));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(lifecycleService), &outParticipantState);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    state = std::promise<SilKit::Services::Orchestration::ParticipantState>{}; // reset state
    state.set_value(SilKit::Services::Orchestration::ParticipantState::Shutdown);

    EXPECT_CALL(mockParticipant.mockLifecycleService, StartLifecycle())
        .Times(testing::Exactly(1))
        .WillOnce(Return(ByMove(state.get_future())));


    returnCode = SilKit_LifecycleService_StartLifecycle((SilKit_LifecycleService*)(lifecycleService));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(
        (SilKit_LifecycleService*)(lifecycleService), &outParticipantState);
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

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Pause(testing::_)
    ).Times(testing::Exactly(1));
    returnCode =
        SilKit_LifecycleService_Pause((SilKit_LifecycleService*)(lifecycleService), "dummy");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
                Stop(testing::_)
    ).Times(testing::Exactly(1));
    returnCode =
        SilKit_LifecycleService_Stop((SilKit_LifecycleService*)(lifecycleService), "dummy");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockParticipant.mockLifecycleService,
        Continue()
    ).Times(testing::Exactly(1));
    returnCode = SilKit_LifecycleService_Continue((SilKit_LifecycleService*)(lifecycleService));
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

}

} // namespace
