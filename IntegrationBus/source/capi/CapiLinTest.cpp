// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/lin/all.hpp"
#include "MockParticipant.hpp"

namespace {
    using namespace ib::sim::lin;
    using ib::mw::test::DummyParticipant;

    class MockLinController : public ib::sim::lin::ILinController {
    public:
        MOCK_METHOD1(Init, void(ControllerConfig config));
        MOCK_METHOD(ControllerStatus, Status, (), (const, noexcept));
        MOCK_METHOD2(SendFrame, void(LinFrame frame, FrameResponseType responseType));
        MOCK_METHOD3(SendFrame, void(LinFrame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp));
        MOCK_METHOD1(SendFrameHeader, void(LinIdT linId));
        MOCK_METHOD2(SendFrameHeader, void(LinIdT linId, std::chrono::nanoseconds timestamp));
        MOCK_METHOD2(SetFrameResponse, void(LinFrame frame, FrameResponseMode mode));
        MOCK_METHOD1(SetFrameResponses, void(std::vector<FrameResponse> responses));
        MOCK_METHOD0(GoToSleep, void());
        MOCK_METHOD0(GoToSleepInternal, void());
        MOCK_METHOD0(Wakeup, void());
        MOCK_METHOD0(WakeupInternal, void());
        MOCK_METHOD1(AddFrameStatusHandler, void(FrameStatusHandler));
        MOCK_METHOD1(AddGoToSleepHandler, void(GoToSleepHandler));
        MOCK_METHOD1(AddWakeupHandler, void(WakeupHandler));
        MOCK_METHOD1(AddFrameResponseUpdateHandler, void(FrameResponseUpdateHandler));
    };

    void CFrameStatusHandler(void* /*context*/, ib_Lin_Controller* /*controller*/,
                             const ib_Lin_FrameStatusEvent* /*frameStatusEvent*/) { }

    void CGoToSleepHandler(void* /*context*/, ib_Lin_Controller* /*controller*/,
                           const ib_Lin_GoToSleepEvent* /*goToSleepEvent*/) { }

    void CWakeupHandler(void* /*context*/, ib_Lin_Controller* /*controller*/, const ib_Lin_WakeupEvent* /*wakeupEvent*/) { }

    class CapiLinTest : public testing::Test
    {
    public:
        CapiLinTest() {}

    protected:
        MockLinController mockController;
        ib::mw::test::DummyParticipant mockParticipant;
    };

    TEST_F(CapiLinTest, lin_controller_function_mapping)
    {
        ib_ReturnCode          returnCode;
        auto                   cMockController = (ib_Lin_Controller*)&mockController;
        ib_Lin_Frame            frame;
        ib_Lin_FrameResponse    frameResponses[1] = {ib_Lin_FrameResponse{}};
        frameResponses[0].frame = &frame;
        auto                   cfg = ib_Lin_ControllerConfig{};
        ib_Lin_ControllerStatus status;

        EXPECT_CALL(mockController, Init(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_Init(cMockController, &cfg);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Status()).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_Status(cMockController, &status);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_SendFrame(cMockController, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrameHeader(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_SendFrameHeader(cMockController, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetFrameResponse(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_SetFrameResponse(cMockController, &frameResponses[0]);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetFrameResponses(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_SetFrameResponses(cMockController, &frameResponses[0], 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, GoToSleep()).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_GoToSleep(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, GoToSleepInternal()).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_GoToSleepInternal(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Wakeup()).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_Wakeup(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, WakeupInternal()).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_WakeupInternal(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameStatusHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_AddFrameStatusHandler(cMockController, nullptr, &CFrameStatusHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddGoToSleepHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_AddGoToSleepHandler(cMockController, nullptr, &CGoToSleepHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddWakeupHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Lin_Controller_AddWakeupHandler(cMockController, nullptr, &CWakeupHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }


    TEST_F(CapiLinTest, lin_controller_nullpointer_params)
    {

        ib_ReturnCode          returnCode;
        auto                   cMockParticipant = (ib_Participant*)&mockParticipant;
        auto                   cMockController = (ib_Lin_Controller*)&mockController;
        ib_Lin_Controller*      linController;
        ib_Lin_Frame            frame;
        ib_Lin_ControllerStatus status;
        auto                   cfg = ib_Lin_ControllerConfig{};
        ib_Lin_FrameResponse    frameResponses[1] = {ib_Lin_FrameResponse{}};

        returnCode = ib_Lin_Controller_Create(&linController, nullptr, "lin", "lin");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_Create(nullptr, cMockParticipant, "lin", "lin");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_Create(&linController, cMockParticipant, nullptr, "lin");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_Create(&linController, cMockParticipant, "lin", nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_Init(nullptr, &cfg);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_Init(cMockController, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_Status(nullptr, &status);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_Status(cMockController, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_SendFrame(nullptr, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_SendFrame(cMockController, nullptr, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_SendFrameHeader(nullptr, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_SetFrameResponse(nullptr, &frameResponses[0]);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_SetFrameResponse(cMockController, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_SetFrameResponses(nullptr, &frameResponses[0], 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_SetFrameResponses(cMockController, nullptr, 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_GoToSleep(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_GoToSleepInternal(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_Wakeup(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_WakeupInternal(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_AddFrameStatusHandler(nullptr, nullptr, &CFrameStatusHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_AddFrameStatusHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_AddGoToSleepHandler(nullptr, nullptr, &CGoToSleepHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_AddGoToSleepHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Lin_Controller_AddWakeupHandler(nullptr, nullptr, &CWakeupHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Lin_Controller_AddWakeupHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}