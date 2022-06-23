/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/sync/all.hpp"

#include "MockParticipant.hpp"

namespace {
    using namespace ib::mw::sync;
    using testing::Return;
    using testing::ByMove;
    using ib::mw::test::DummyParticipant;

    void Create_StringList(ib_StringList** outStringList, const char** strings, uint32_t numStrings)
    {
        ib_StringList* newStrings;
        newStrings = (ib_StringList*)malloc(sizeof(ib_StringList));
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
        ib::mw::test::DummyParticipant mockParticipant;

        CapiParticipantStateHandlingTest() 
        {
            uint32_t numNames = 2;
            const char* names[2] = {"Participant1", "Participant2"};
            Create_StringList(&participantNames, names, numNames);
        }

        ib_StringList* participantNames;

	};

    void InitCallback(void* /*context*/, ib_Participant* /*participant*/) {}
    void StopCallback(void* /*context*/, ib_Participant* /*participant*/) {}
    void ShutdownCallback(void* /*context*/, ib_Participant* /*participant*/) {}
    void SystemStateHandler(void* /*context*/, ib_Participant* /*participant*/, ib_SystemState /*state*/) {}

    TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_nullpointer_params)
    {
        ib_ReturnCode returnCode;

        returnCode = ib_Participant_SetInitHandler(nullptr, NULL, &InitCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_SetInitHandler((ib_Participant*)&mockParticipant, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_SetStopHandler(nullptr, NULL, &StopCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_SetStopHandler((ib_Participant*)&mockParticipant, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_SetShutdownHandler(nullptr, NULL, &ShutdownCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_SetShutdownHandler((ib_Participant*)&mockParticipant, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // ExecuteLifecycleNoSyncTime
        ib_ParticipantState outParticipantState;
        returnCode = ib_Participant_ExecuteLifecycleNoSyncTime(nullptr,
            ib_False, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode = ib_Participant_ExecuteLifecycleNoSyncTime((ib_Participant*)&mockParticipant,
            0xcd, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // ExecuteLifecycleWithSyncTime
        returnCode = ib_Participant_ExecuteLifecycleWithSyncTime(nullptr,
            ib_False, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_ExecuteLifecycleWithSyncTime((ib_Participant*)&mockParticipant,
            0xcd, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // WaitForLifecycleToComplete
        returnCode = ib_Participant_WaitForLifecycleToComplete(nullptr, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_WaitForLifecycleToComplete((ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Initialize(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Initialize((ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Initialize(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Restart((ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Restart(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Restart(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_RunSimulation(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_StopSimulation(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Pause(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Pause((ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Pause(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_Continue(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_PrepareColdswap(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_ExecuteColdswap(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_ParticipantState participantState;
        returnCode = ib_Participant_GetParticipantState(nullptr, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(nullptr, (ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(nullptr, (ib_Participant*)&mockParticipant, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(nullptr, nullptr, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(&participantState, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(&participantState, (ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetParticipantState(&participantState, nullptr, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_SystemState systemState;
        returnCode = ib_Participant_GetSystemState(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetSystemState(&systemState, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_GetSystemState(nullptr, (ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // NOTE 2nd parameter 'context' is optional and may be nullptr
        returnCode = ib_Participant_RegisterSystemStateHandler(nullptr, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_RegisterSystemStateHandler(nullptr, nullptr, &SystemStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_RegisterSystemStateHandler((ib_Participant*)&mockParticipant, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_SetRequiredParticipants(nullptr, participantNames);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_SetRequiredParticipants((ib_Participant*)&mockParticipant, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    }

    TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
    {
        ib_ReturnCode returnCode;
        auto* cParticipant = (ib_Participant*)&mockParticipant;

        // required for MockSystemMonitor::ParticipantStatus
        testing::DefaultValue<const ib::mw::sync::ParticipantStatus&>::Set(ib::mw::sync::ParticipantStatus());

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            SetCommunicationReadyHandler(testing::_)
        ).Times(testing::Exactly(1));

        returnCode = ib_Participant_SetInitHandler(
            cParticipant,
            nullptr,
            &InitCallback);

        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            SetStopHandler(testing::_)
        ).Times(testing::Exactly(1));

        returnCode = ib_Participant_SetStopHandler(
            cParticipant,
            nullptr,
            &StopCallback);

        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            SetShutdownHandler(testing::_)
        ).Times(testing::Exactly(1));
        returnCode = ib_Participant_SetShutdownHandler(
            cParticipant,
            nullptr,
            &ShutdownCallback);

        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        ///////////////////////////////////////////////
        // Lifecycle tests
        ///////////////////////////////////////////////

        using testing::_;
        ib_ParticipantState outParticipantState{ib_ParticipantState_Invalid};
        std::promise<ParticipantState> state;
        state.set_value(ParticipantState::Shutdown);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            ExecuteLifecycleWithSyncTime(_, _, _, _)
        ).Times(testing::Exactly(1))
            .WillOnce(Return(ByMove(state.get_future())));


        returnCode = ib_Participant_ExecuteLifecycleWithSyncTime(
            cParticipant, ib_False, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
        returnCode = ib_Participant_WaitForLifecycleToComplete(cParticipant, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        state = std::promise<ParticipantState> {}; // reset state
        state.set_value(ParticipantState::Shutdown);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            ExecuteLifecycleNoSyncTime(_, _, _)
        ).Times(testing::Exactly(1))
            .WillOnce(Return(ByMove(state.get_future())));


        returnCode = ib_Participant_ExecuteLifecycleNoSyncTime(
            cParticipant, ib_False, ib_False, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
        returnCode = ib_Participant_WaitForLifecycleToComplete(cParticipant, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        ib_SystemState systemState;
        EXPECT_CALL(mockParticipant.mockSystemMonitor, SystemState()).Times(testing::Exactly(1));
        returnCode = ib_Participant_GetSystemState(&systemState, (ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        ib_ParticipantState participantState;
        EXPECT_CALL(mockParticipant.mockSystemMonitor, ParticipantStatus(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Participant_GetParticipantState(&participantState, (ib_Participant*)&mockParticipant, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        returnCode = ib_Participant_Initialize((ib_Participant*)&mockParticipant, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        returnCode = ib_Participant_Restart((ib_Participant*)&mockParticipant, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, Run()).Times(testing::Exactly(1));
        returnCode = ib_Participant_RunSimulation((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            Pause(testing::_)
        ).Times(testing::Exactly(1));
        returnCode = ib_Participant_Pause((ib_Participant*)&mockParticipant, "dummy");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockLifecycleService,
            Continue()
        ).Times(testing::Exactly(1));
        returnCode = ib_Participant_Continue((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, Stop()).Times(testing::Exactly(1));
        returnCode = ib_Participant_StopSimulation((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, PrepareColdswap()).Times(testing::Exactly(1));
        returnCode = ib_Participant_PrepareColdswap((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, ExecuteColdswap()).Times(testing::Exactly(1));
        returnCode = ib_Participant_ExecuteColdswap((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, Shutdown()).Times(testing::Exactly(1));
        returnCode = ib_Participant_Shutdown((ib_Participant*)&mockParticipant);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockParticipant.mockSystemController, SetRequiredParticipants(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Participant_SetRequiredParticipants((ib_Participant*)&mockParticipant, participantNames);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    }

}