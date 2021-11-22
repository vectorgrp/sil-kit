#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/sync/all.hpp"

#include "MockComAdapter.hpp"

namespace {
    using namespace ib::mw::sync;
    using ib::mw::test::DummyComAdapter;

	class CapiTimeSyncTest : public testing::Test
	{
	protected: 
        ib::mw::test::DummyComAdapter mockComAdapter;

        CapiTimeSyncTest() {}

	};

    void SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now) {}

    TEST_F(CapiTimeSyncTest, participant_state_handling_nullpointer_params)
    {
        ib_ReturnCode returnCode;
        returnCode = ib_SimulationParticipant_SetPeriod(nullptr, 1000);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_SetSimulationTask(nullptr, NULL, &SimTask);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_SetSimulationTask((ib_SimulationParticipant*)&mockComAdapter, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

    TEST_F(CapiTimeSyncTest, participant_state_handling_function_mapping)
    {
        ib_ReturnCode returnCode;
        EXPECT_CALL(mockComAdapter.mockParticipantController, SetPeriod(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetPeriod((ib_SimulationParticipant*)&mockComAdapter, 1000);

        EXPECT_CALL(mockComAdapter.mockParticipantController, SetSimulationTask(testing::Matcher<ib::mw::sync::IParticipantController::SimTaskT>(testing::_))).Times(testing::Exactly(1));
        returnCode = ib_SimulationParticipant_SetSimulationTask((ib_SimulationParticipant*)&mockComAdapter, NULL, &SimTask);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

}