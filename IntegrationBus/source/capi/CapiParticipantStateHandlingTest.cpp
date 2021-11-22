#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/sync/all.hpp"

#include "MockComAdapter.hpp"

namespace {
    using namespace ib::mw::sync;
    using ib::mw::test::DummyComAdapter;

	class CapiParticipantStateHandlingTest : public testing::Test
	{
	protected: 
        ib::mw::test::DummyComAdapter mockComAdapter;

        CapiParticipantStateHandlingTest() {}

	};

    void InitCallback(void* context, ib_SimulationParticipant* participant,
        ib_ParticipantCommand* command) {}
    void StopCallback(void* context, ib_SimulationParticipant* participant) {}
    void ShutdownCallback(void* context, ib_SimulationParticipant* participant) {}

    void SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now) {}

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
    }

    TEST_F(CapiParticipantStateHandlingTest, participant_state_handling_function_mapping)
    {
        ib_ReturnCode returnCode;
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
    }

}