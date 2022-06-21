#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/sync/all.hpp"

#include "MockParticipant.hpp"

namespace {
    using namespace ib::mw::sync;
    using ib::mw::test::DummyParticipant;

	class CapiTimeSyncTest : public testing::Test
	{
	protected: 
        ib::mw::test::DummyParticipant mockParticipant;

        CapiTimeSyncTest() {}

	};

    void SimTask(void* /*context*/, ib_Participant* /*participant*/, ib_NanosecondsTime /*now*/) {}

    TEST_F(CapiTimeSyncTest, participant_state_handling_nullpointer_params)
    {
        ib_ReturnCode returnCode;
        returnCode = ib_Participant_SetPeriod(nullptr, 1000);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Participant_SetSimulationTask(nullptr, nullptr, &SimTask);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_SetSimulationTask((ib_Participant*)&mockParticipant, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

    TEST_F(CapiTimeSyncTest, participant_state_handling_function_mapping)
    {
        ib_ReturnCode returnCode;
        EXPECT_CALL(mockParticipant.mockTimeSyncService,
            SetPeriod(testing::_)
        ).Times(testing::Exactly(1));
        returnCode = ib_Participant_SetPeriod((ib_Participant*)&mockParticipant, 1000);

        EXPECT_CALL(mockParticipant.mockTimeSyncService,
            SetSimulationTask(testing::Matcher<ib::mw::sync::ITimeSyncService::SimTaskT>(testing::_))
        ).Times(testing::Exactly(1));
        returnCode = ib_Participant_SetSimulationTask((ib_Participant*)&mockParticipant, nullptr, &SimTask);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

}