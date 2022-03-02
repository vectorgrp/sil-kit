#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/sync/all.hpp"

#include "MockComAdapter.hpp"

namespace {
    using namespace ib::mw::sync;
    using ib::mw::test::DummyComAdapter;

    void Create_StringList(ib_StringList** outStringList, const char** strings, uint32_t numStrings)
    {
        ib_StringList* newStrings;
        size_t stringsSize = numStrings * sizeof(char*);
        size_t stringListSize = sizeof(ib_StringList) + stringsSize;
        newStrings = (ib_StringList*)malloc(stringListSize);
        if (newStrings != nullptr)
        {
            newStrings->numStrings = numStrings;
            for (uint32_t i = 0; i < numStrings; i++)
            {
                auto len = strlen(strings[i]) + 1;
                newStrings->strings[i] = (char*)malloc(len);
                if (newStrings->strings[i] != nullptr)
                {
                    strcpy((char*)newStrings->strings[i], strings[i]);
                }
            }
        }
        *outStringList = newStrings;
    }

	class CapiParticipantStateHandlingTest : public testing::Test
	{
	protected: 
        ib::mw::test::DummyComAdapter mockComAdapter;

        CapiParticipantStateHandlingTest() 
        {
            uint32_t numNames = 2;
            const char* names[2] = {"Participant1", "Participant2"};
            Create_StringList(&participantNames, names, numNames);
        }

        ib_StringList* participantNames;

	};

    void InitCallback(void* context, ib_SimulationParticipant* participant,
        ib_ParticipantCommand* command) {}
    void StopCallback(void* context, ib_SimulationParticipant* participant) {}
    void ShutdownCallback(void* context, ib_SimulationParticipant* participant) {}

    void SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now) {}
    void SystemStateHandler(void* context, ib_SimulationParticipant* participant, ib_SystemState state) {}
    void ParticipantStateHandler(void* context, ib_SimulationParticipant* participant, const char* participantId, ib_ParticipantState state) {}

    TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_nullpointer_params)
    {
        ib_ReturnCode returnCode;

        returnCode = ib_SimulationParticipant_SetInitHandler(nullptr, NULL, &InitCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_SetInitHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_SetStopHandler(nullptr, NULL, &StopCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_SetStopHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_SetShutdownHandler(nullptr, NULL, &ShutdownCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_SetShutdownHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_ParticipantState outParticipantState;
        returnCode = ib_SimulationParticipant_Run(nullptr, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Run((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_RunAsync(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_WaitForRunAsyncToComplete(nullptr, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_WaitForRunAsyncToComplete((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // Expect error when not calling RunAsync with a valid ib_SimulationParticipant before
        returnCode = ib_SimulationParticipant_WaitForRunAsyncToComplete((ib_SimulationParticipant*)&mockComAdapter, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Initialize(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Initialize((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Initialize(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_ReInitialize((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_ReInitialize(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_ReInitialize(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_RunSimulation(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_StopSimulation(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Pause(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Pause((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Pause(nullptr, "test");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_Continue(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_PrepareColdswap(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_ExecuteColdswap(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_ParticipantState participantState;
        returnCode = ib_SimulationParticipant_GetParticipantState(nullptr, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(nullptr, (ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(nullptr, (ib_SimulationParticipant*)&mockComAdapter, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(nullptr, nullptr, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(&participantState, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(&participantState, (ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetParticipantState(&participantState, nullptr, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_SystemState systemState;
        returnCode = ib_SimulationParticipant_GetSystemState(nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetSystemState(&systemState, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_GetSystemState(nullptr, (ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        // NOTE 2nd parameter 'context' is optional and may be nullptr
        returnCode = ib_SimulationParticipant_RegisterSystemStateHandler(nullptr, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_RegisterSystemStateHandler(nullptr, nullptr, &SystemStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_RegisterSystemStateHandler((ib_SimulationParticipant*)&mockComAdapter, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_SetRequiredParticipants(nullptr, participantNames);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_SetRequiredParticipants((ib_SimulationParticipant*)&mockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    }

    TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
    {
        ib_ReturnCode returnCode;

        // required for MockSystemMonitor::ParticipantStatus
        testing::DefaultValue<const ib::mw::sync::ParticipantStatus&>::Set(ib::mw::sync::ParticipantStatus());

        EXPECT_CALL(mockComAdapter.mockParticipantController, SetInitHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetInitHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, &InitCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, SetStopHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetStopHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, &StopCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, SetShutdownHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetShutdownHandler((ib_SimulationParticipant*)&mockComAdapter, NULL, &ShutdownCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, Run()).Times(testing::Exactly(1));
        ib_ParticipantState outParticipantState;
        returnCode = ib_SimulationParticipant_Run((ib_SimulationParticipant*)&mockComAdapter, &outParticipantState);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, RunAsync()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_RunAsync((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        returnCode = ib_SimulationParticipant_RunAsync((ib_SimulationParticipant*)&mockComAdapter); // Second call should fail
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_SystemState systemState;
        EXPECT_CALL(mockComAdapter.mockSystemMonitor, SystemState()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_GetSystemState(&systemState, (ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        ib_ParticipantState participantState;
        EXPECT_CALL(mockComAdapter.mockSystemMonitor, ParticipantStatus(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_GetParticipantState(&participantState, (ib_SimulationParticipant*)&mockComAdapter, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        returnCode = ib_SimulationParticipant_Initialize((ib_SimulationParticipant*)&mockComAdapter, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        returnCode = ib_SimulationParticipant_ReInitialize((ib_SimulationParticipant*)&mockComAdapter, "participant");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, Run()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_RunSimulation((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, Pause(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_Pause((ib_SimulationParticipant*)&mockComAdapter, "dummy");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockParticipantController, Continue()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_Continue((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, Stop()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_StopSimulation((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, PrepareColdswap()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_PrepareColdswap((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, ExecuteColdswap()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_ExecuteColdswap((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, Shutdown()).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_Shutdown((ib_SimulationParticipant*)&mockComAdapter);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockComAdapter.mockSystemController, SetRequiredParticipants(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetRequiredParticipants((ib_SimulationParticipant*)&mockComAdapter, participantNames);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    }

}