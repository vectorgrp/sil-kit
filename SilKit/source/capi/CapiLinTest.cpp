// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/lin/all.hpp"
#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::Lin;
using SilKit::Core::Tests::DummyParticipant;

class MockLinController : public SilKit::Services::Lin::ILinController {
public:
    MOCK_METHOD1(Init, void(LinControllerConfig config));
    MOCK_METHOD(LinControllerStatus, Status, (), (const, noexcept));
    MOCK_METHOD2(SendFrame, void(LinFrame frame, LinFrameResponseType responseType));
    MOCK_METHOD3(SendFrame, void(LinFrame frame, LinFrameResponseType responseType, std::chrono::nanoseconds timestamp));
    MOCK_METHOD1(SendFrameHeader, void(LinIdT linId));
    MOCK_METHOD2(SendFrameHeader, void(LinIdT linId, std::chrono::nanoseconds timestamp));
    MOCK_METHOD2(SetFrameResponse, void(LinFrame frame, LinFrameResponseMode mode));
    MOCK_METHOD1(SetFrameResponses, void(std::vector<LinFrameResponse> responses));
    MOCK_METHOD0(GoToSleep, void());
    MOCK_METHOD0(GoToSleepInternal, void());
    MOCK_METHOD0(Wakeup, void());
    MOCK_METHOD0(WakeupInternal, void());

    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameStatusHandler, (FrameStatusHandler));
    MOCK_METHOD(void, RemoveFrameStatusHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddGoToSleepHandler, (GoToSleepHandler));
    MOCK_METHOD(void, RemoveGoToSleepHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddWakeupHandler, (WakeupHandler));
    MOCK_METHOD(void, RemoveWakeupHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameResponseUpdateHandler, (FrameResponseUpdateHandler));
    MOCK_METHOD(void, RemoveFrameResponseUpdateHandler, (SilKit::Services::HandlerId));
};

void CFrameStatusHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                         const SilKit_LinFrameStatusEvent* /*frameStatusEvent*/) { }

void CGoToSleepHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                       const SilKit_LinGoToSleepEvent* /*goToSleepEvent*/) { }

void CWakeupHandler(void* /*context*/, SilKit_LinController* /*controller*/, const SilKit_LinWakeupEvent* /*wakeupEvent*/) { }

class CapiLinTest : public testing::Test
{
public:
    CapiLinTest() {}

protected:
    MockLinController mockController;
    SilKit::Core::Tests::DummyParticipant mockParticipant;
};

TEST_F(CapiLinTest, lin_controller_function_mapping)
{
    using SilKit::Util::HandlerId;

    SilKit_ReturnCode returnCode;
    auto cMockController = (SilKit_LinController*)&mockController;
    SilKit_LinFrame frame{};
    SilKit_LinFrameResponse frameResponses[1] = {SilKit_LinFrameResponse{}};
    frameResponses[0].frame = &frame;

    SilKit_Struct_Init(SilKit_LinFrame, frame);
    SilKit_Struct_Init(SilKit_LinFrameResponse, frameResponses[0]);

    auto cfg = SilKit_LinControllerConfig{};
    SilKit_Struct_Init(SilKit_LinControllerConfig, cfg);
    SilKit_LinControllerStatus status{};
    SilKit_HandlerId handlerId{};

    EXPECT_CALL(mockController, Init(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_Init(cMockController, &cfg);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Status()).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_Status(cMockController, &status);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_SendFrame(cMockController, &frame, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SendFrameHeader(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_SendFrameHeader(cMockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SetFrameResponse(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_SetFrameResponse(cMockController, &frameResponses[0]);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SetFrameResponses(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_SetFrameResponses(cMockController, &frameResponses[0], 1);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, GoToSleep()).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_GoToSleep(cMockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, GoToSleepInternal()).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_GoToSleepInternal(cMockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Wakeup()).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_Wakeup(cMockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, WakeupInternal()).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_WakeupInternal(cMockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode =
        SilKit_LinController_AddFrameStatusHandler(cMockController, nullptr, &CFrameStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameStatusHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_RemoveFrameStatusHandler((SilKit_LinController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddGoToSleepHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_AddGoToSleepHandler(cMockController, nullptr, &CGoToSleepHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveGoToSleepHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_RemoveGoToSleepHandler((SilKit_LinController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddWakeupHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_AddWakeupHandler(cMockController, nullptr, &CWakeupHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveWakeupHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_RemoveWakeupHandler((SilKit_LinController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiLinTest, lin_controller_nullpointer_params)
{
    SilKit_HandlerId handlerId;
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    auto cMockController = (SilKit_LinController*)&mockController;
    SilKit_LinController* linController;
    SilKit_LinFrame frame;
    SilKit_LinControllerStatus status;
    auto cfg = SilKit_LinControllerConfig{};
    SilKit_LinFrameResponse frameResponses[1] = {SilKit_LinFrameResponse{}};

    returnCode = SilKit_LinController_Create(&linController, nullptr, "lin", "lin");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_Create(nullptr, cMockParticipant, "lin", "lin");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_Create(&linController, cMockParticipant, nullptr, "lin");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_Create(&linController, cMockParticipant, "lin", nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_Init(nullptr, &cfg);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_Init(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_Status(nullptr, &status);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_Status(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_SendFrame(nullptr, &frame, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_SendFrame(cMockController, nullptr, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_SendFrameHeader(nullptr, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_SetFrameResponse(nullptr, &frameResponses[0]);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_SetFrameResponse(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_SetFrameResponses(nullptr, &frameResponses[0], 1);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_SetFrameResponses(cMockController, nullptr, 1);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_GoToSleep(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_GoToSleepInternal(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_Wakeup(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_WakeupInternal(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_AddFrameStatusHandler(nullptr, nullptr, &CFrameStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_AddFrameStatusHandler(cMockController, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode =
        SilKit_LinController_AddFrameStatusHandler(cMockController, nullptr, &CFrameStatusHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_AddGoToSleepHandler(nullptr, nullptr, &CGoToSleepHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_AddGoToSleepHandler(cMockController, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_AddGoToSleepHandler(cMockController, nullptr, &CGoToSleepHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_AddWakeupHandler(nullptr, nullptr, &CWakeupHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_AddWakeupHandler(cMockController, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_AddWakeupHandler(cMockController, nullptr, &CWakeupHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

}