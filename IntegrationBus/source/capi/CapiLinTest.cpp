// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/lin/all.hpp"
#include "MockComAdapter.hpp"

namespace {
    using namespace ib::sim::lin;
    using ib::mw::test::DummyComAdapter;

    class MockLinController : public ib::sim::lin::ILinController {
    public:
        MOCK_METHOD1(Init, void(ControllerConfig config));
        MOCK_METHOD(ControllerStatus, Status, (), (const, noexcept));
        MOCK_METHOD2(SendFrame, void(Frame frame, FrameResponseType responseType));
        MOCK_METHOD3(SendFrame, void(Frame frame, FrameResponseType responseType, std::chrono::nanoseconds timestamp));
        MOCK_METHOD1(SendFrameHeader, void(LinIdT linId));
        MOCK_METHOD2(SendFrameHeader, void(LinIdT linId, std::chrono::nanoseconds timestamp));
        MOCK_METHOD2(SetFrameResponse, void(Frame frame, FrameResponseMode mode));
        MOCK_METHOD1(SetFrameResponses, void(std::vector<FrameResponse> responses));
        MOCK_METHOD0(GoToSleep, void());
        MOCK_METHOD0(GoToSleepInternal, void());
        MOCK_METHOD0(Wakeup, void());
        MOCK_METHOD0(WakeupInternal, void());
        MOCK_METHOD1(RegisterFrameStatusHandler, void(FrameStatusHandler));
        MOCK_METHOD1(RegisterGoToSleepHandler, void(GoToSleepHandler));
        MOCK_METHOD1(RegisterWakeupHandler, void(WakeupHandler));
        MOCK_METHOD1(RegisterFrameResponseUpdateHandler, void(FrameResponseUpdateHandler));
    };

    void CFrameStatusHandler(void* context, ib_LinController* controller, const ib_LinFrame* frame,
                             ib_LinFrameStatus status, ib_NanosecondsTime timestamp) { }

    void CGoToSleepHandler(void* context, ib_LinController* controller) { }

    void CWakeupHandler(void* context, ib_LinController* controller) { }

    class CapiLinTest : public testing::Test
    {
    public:
        CapiLinTest() {}

    protected:
        MockLinController mockController;
        ib::mw::test::DummyComAdapter mockComAdapter;
    };

    TEST_F(CapiLinTest, lin_controller_function_mapping)
    {
        ib_ReturnCode          returnCode;
        auto                   cMockController = (ib_LinController*)&mockController;
        ib_LinFrame            frame;
        ib_LinFrameResponse    frameResponses[1] = {ib_LinFrameResponse{}};
        auto                   cfg = ib_LinControllerConfig{};
        ib_LinControllerStatus status;

        EXPECT_CALL(mockController, Init(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_Init(cMockController, &cfg);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Status()).Times(testing::Exactly(1));
        returnCode = ib_LinController_Status(cMockController, &status);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SendFrame(cMockController, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SendFrameWithTimestamp(cMockController, &frame, 0, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrameHeader(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SendFrameHeader(cMockController, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrameHeader(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SendFrameHeaderWithTimestamp(cMockController, 0, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetFrameResponse(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SetFrameResponse(cMockController, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetFrameResponses(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_SetFrameResponses(cMockController, &frameResponses[0], 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, GoToSleep()).Times(testing::Exactly(1));
        returnCode = ib_LinController_GoToSleep(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, GoToSleepInternal()).Times(testing::Exactly(1));
        returnCode = ib_LinController_GoToSleepInternal(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Wakeup()).Times(testing::Exactly(1));
        returnCode = ib_LinController_Wakeup(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, WakeupInternal()).Times(testing::Exactly(1));
        returnCode = ib_LinController_WakeupInternal(cMockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterFrameStatusHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_RegisterFrameStatusHandler(cMockController, nullptr, &CFrameStatusHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterGoToSleepHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_RegisterGoToSleepHandler(cMockController, nullptr, &CGoToSleepHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterWakeupHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_LinController_RegisterWakeupHandler(cMockController, nullptr, &CWakeupHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }


    TEST_F(CapiLinTest, lin_controller_nullpointer_params)
    {

        ib_ReturnCode          returnCode;
        auto                   cMockComAdapter = (ib_SimulationParticipant*)&mockComAdapter;
        auto                   cMockController = (ib_LinController*)&mockController;
        ib_LinController*      linController;
        ib_LinFrame            frame;
        ib_LinControllerStatus status;
        auto                   cfg = ib_LinControllerConfig{};
        ib_LinFrameResponse    frameResponses[1] = {ib_LinFrameResponse{}};

        returnCode = ib_LinController_create(&linController, nullptr, "lin");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_create(nullptr, cMockComAdapter, "lin");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_create(&linController, cMockComAdapter, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_Init(nullptr, &cfg);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_Init(cMockController, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_Status(nullptr, &status);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_Status(cMockController, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SendFrame(nullptr, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_SendFrame(cMockController, nullptr, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SendFrameWithTimestamp(nullptr, &frame, 0, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_SendFrameWithTimestamp(cMockController, nullptr, 0, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SendFrameHeader(nullptr, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SendFrameHeaderWithTimestamp(nullptr, 0, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SetFrameResponse(nullptr, &frame, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_SetFrameResponse(cMockController, nullptr, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_SetFrameResponses(nullptr, &frameResponses[0], 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_SetFrameResponses(cMockController, nullptr, 1);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_GoToSleep(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_GoToSleepInternal(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_Wakeup(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_WakeupInternal(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_RegisterFrameStatusHandler(nullptr, nullptr, &CFrameStatusHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_RegisterFrameStatusHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_RegisterGoToSleepHandler(nullptr, nullptr, &CGoToSleepHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_RegisterGoToSleepHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_LinController_RegisterWakeupHandler(nullptr, nullptr, &CWakeupHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_LinController_RegisterWakeupHandler(cMockController, nullptr, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}