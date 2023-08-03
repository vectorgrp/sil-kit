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
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"
#include "silkit/services/lin/all.hpp"

#include "MockParticipant.hpp"
#include "ILinControllerExtensions.hpp"

namespace {
using namespace SilKit::Services::Lin;
using SilKit::Core::Tests::DummyParticipant;

class MockLinController
    : public SilKit::Services::Lin::ILinController
    , public SilKit::Services::Lin::ILinControllerExtensions
{
public:
    MOCK_METHOD(void, Init, (LinControllerConfig config), (override));
    MOCK_METHOD(void, InitDynamic,(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& config), (override));
    MOCK_METHOD(LinControllerStatus, Status, (), (const, noexcept));
    MOCK_METHOD(void, SendFrame, (LinFrame frame, LinFrameResponseType responseType));
    MOCK_METHOD(void, SendFrame, (LinFrame frame, LinFrameResponseType responseType, std::chrono::nanoseconds timestamp));
    MOCK_METHOD(void, SendFrameHeader, (LinId linId), (override));
    MOCK_METHOD(void, SendFrameHeader, (LinId linId, std::chrono::nanoseconds timestamp));
    MOCK_METHOD(void, UpdateTxBuffer, (LinFrame frame), (override));
    MOCK_METHOD(void, SetFrameResponse, (LinFrameResponse response), (override));
    MOCK_METHOD(void, SendDynamicResponse, (const LinFrame& frame), (override));
    MOCK_METHOD0(GoToSleep, void());
    MOCK_METHOD0(GoToSleepInternal, void());
    MOCK_METHOD0(Wakeup, void());
    MOCK_METHOD0(WakeupInternal, void());
    MOCK_METHOD(SilKit::Experimental::Services::Lin::LinSlaveConfiguration, GetSlaveConfiguration, (), (override));

    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameStatusHandler, (FrameStatusHandler));
    MOCK_METHOD(void, RemoveFrameStatusHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddGoToSleepHandler, (GoToSleepHandler));
    MOCK_METHOD(void, RemoveGoToSleepHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddWakeupHandler, (WakeupHandler));
    MOCK_METHOD(void, RemoveWakeupHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(void, RemoveFrameHeaderHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddLinSlaveConfigurationHandler,
                (SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler), (override));
    MOCK_METHOD(void, RemoveLinSlaveConfigurationHandler, (SilKit::Services::HandlerId), (override));

    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHeaderHandler, (SilKit::Experimental::Services::Lin::LinFrameHeaderHandler), (override));
};

void SilKitCALL CFrameStatusHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                         const SilKit_LinFrameStatusEvent* /*frameStatusEvent*/) { }

void SilKitCALL CGoToSleepHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                       const SilKit_LinGoToSleepEvent* /*goToSleepEvent*/) { }

void SilKitCALL CWakeupHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                    const SilKit_LinWakeupEvent* /*wakeupEvent*/) { }

void SilKitCALL CFrameHeaderHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                                    const SilKit_Experimental_LinFrameHeaderEvent* /*frameHeaderEvent*/) { }

void SilKitCALL CLinSlaveConfigurationHandler(void* /*context*/, SilKit_LinController* /*controller*/,
                                   const SilKit_Experimental_LinSlaveConfigurationEvent* /*slaveConfigurationEvent*/) { }

class Test_CapiLin : public testing::Test
{
public:
    Test_CapiLin() {}

protected:
    MockLinController mockController;
    SilKit::Core::Tests::DummyParticipant mockParticipant;
};

TEST_F(Test_CapiLin, lin_controller_function_mapping)
{
    using SilKit::Util::HandlerId;

    SilKit_ReturnCode returnCode;
    auto cMockController = (SilKit_LinController*)&mockController;
    SilKit_LinFrame frame{};
    SilKit_Struct_Init(SilKit_LinFrame, frame);

    auto cfg = SilKit_LinControllerConfig{};
    SilKit_Struct_Init(SilKit_LinControllerConfig, cfg);
    SilKit_LinControllerStatus status{};
    SilKit_HandlerId handlerId{};
    SilKit_LinFrameResponse response{};
    SilKit_Struct_Init(SilKit_LinFrameResponse, response);
    response.frame = &frame;

    SilKit_Experimental_LinControllerDynamicConfig dynamicConfig{};
    SilKit_Struct_Init(SilKit_Experimental_LinControllerDynamicConfig, dynamicConfig);

    EXPECT_CALL(mockController, Init(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_Init(cMockController, &cfg);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SetFrameResponse(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_SetFrameResponse(cMockController, &response);
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

    EXPECT_CALL(mockController, UpdateTxBuffer(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_LinController_UpdateTxBuffer(cMockController, &frame);
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

    EXPECT_CALL(mockController, AddLinSlaveConfigurationHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(cMockController, nullptr, &CLinSlaveConfigurationHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveLinSlaveConfigurationHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler((SilKit_LinController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHeaderHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_AddFrameHeaderHandler((SilKit_LinController*)&mockController, nullptr, &CFrameHeaderHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameHeaderHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_RemoveFrameHeaderHandler((SilKit_LinController*)&mockController, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, InitDynamic(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_InitDynamic((SilKit_LinController*)&mockController, &dynamicConfig);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, SendDynamicResponse(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_SendDynamicResponse((SilKit_LinController*)&mockController, &frame);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHeaderHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_AddFrameHeaderHandler((SilKit_LinController*)&mockController, nullptr, &CFrameHeaderHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameHeaderHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_Experimental_LinController_RemoveFrameHeaderHandler((SilKit_LinController*)&mockController, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiLin, lin_controller_nullpointer_params)
{
    SilKit_HandlerId handlerId;
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    auto cMockController = (SilKit_LinController*)&mockController;
    SilKit_LinController* linController;
    SilKit_LinFrame frame;
    SilKit_LinControllerStatus status;
    auto cfg = SilKit_LinControllerConfig{};
    SilKit_Experimental_LinSlaveConfiguration linSlaveConfiguration;
    SilKit_LinFrameResponse response{};

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

    returnCode = SilKit_LinController_SetFrameResponse(nullptr, &response);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_SetFrameResponse(cMockController, nullptr);
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

    returnCode = SilKit_LinController_UpdateTxBuffer(nullptr, &frame);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_LinController_UpdateTxBuffer(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_GoToSleep(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_GoToSleepInternal(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_Wakeup(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_LinController_WakeupInternal(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Experimental_LinController_GetSlaveConfiguration(nullptr, &linSlaveConfiguration);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_GetSlaveConfiguration(cMockController, nullptr);
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

    returnCode = SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(nullptr, nullptr, &CLinSlaveConfigurationHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(cMockController, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(cMockController, nullptr, &CLinSlaveConfigurationHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Experimental_LinController_InitDynamic(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_InitDynamic(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Experimental_LinController_SendDynamicResponse(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_SendDynamicResponse(cMockController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Experimental_LinController_AddFrameHeaderHandler(nullptr, nullptr, &CFrameHeaderHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_AddFrameHeaderHandler(cMockController, nullptr, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_Experimental_LinController_AddFrameHeaderHandler(cMockController, nullptr, &CFrameHeaderHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Experimental_LinController_RemoveFrameHeaderHandler(nullptr, static_cast<SilKit_HandlerId>(0));
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

}
