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

#include "silkit/SilKit.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

using namespace SilKit::Services::Lin;

MATCHER_P(LinControllerConfigMatcher, controlFrame, "")
{
    *result_listener << "matches Lin frames of the c-api to the cpp api";

    LinControllerConfig config1 = controlFrame;
    const SilKit_LinControllerConfig* config2 = arg;

    if (config1.baudRate != config2->baudRate
        || static_cast<uint8_t>(config1.controllerMode) != static_cast<uint8_t>(config2->controllerMode))
    {
        return false;
    }

    for (size_t i = 0; i < config1.frameResponses.size(); i++)
    {
        if (static_cast<uint8_t>(config1.frameResponses[i].responseMode)
            != static_cast<uint8_t>(config2->frameResponses[i].responseMode))
        {
            return false;
        }
        if (config1.frameResponses[i].frame.id != config2->frameResponses[i].frame->id)
        {
            return false;
        }
        if (config1.frameResponses[i].frame.dataLength != config2->frameResponses[i].frame->dataLength)
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(LinControllerDynamicConfigMatcher, controlFrame, "")
{
    *result_listener << "matches LinControllerDynamicConfig of the c-api to the cpp api";

    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config1 = controlFrame;
    const SilKit_Experimental_LinControllerDynamicConfig* config2 = arg;

    const auto sameControllerMode =
        static_cast<uint8_t>(config1.controllerMode) == static_cast<uint8_t>(config2->controllerMode);
    if ((config1.baudRate != config2->baudRate) || !sameControllerMode)
    {
        return false;
    }

    return true;
}

MATCHER_P(LinFrameMatcher, controlFrame, "")
{
    *result_listener << "matches Lin frames of the c-api to the cpp api";

    LinFrame frame1 = controlFrame;
    const SilKit_LinFrame* frame2 = arg;

    if (static_cast<uint8_t>(frame1.checksumModel) != static_cast<uint8_t>(frame2->checksumModel)
        || static_cast<uint8_t>(frame1.dataLength) != static_cast<uint8_t>(frame2->dataLength)
        || static_cast<uint8_t>(frame1.id) != static_cast<uint8_t>(frame2->id))
    {
        return false;
    }

    for (size_t i = 0; i < frame1.data.size(); i++)
    {
        if (static_cast<uint8_t>(frame1.data[i]) != static_cast<uint8_t>(frame2->data[i]))
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(LinFrameResponseMatcher, controlFrameResponse, "")
{
    *result_listener << "matches Lin frame responses of the c-api to the cpp api";

    LinFrameResponse frameResponse1 = controlFrameResponse;
    const SilKit_LinFrameResponse* frameResponse2 = arg;

    if (static_cast<SilKit_LinFrameResponseMode>(frameResponse1.responseMode) != frameResponse2->responseMode)
    {
        return false;
    }

    return testing::Matches(LinFrameMatcher(frameResponse1.frame))(frameResponse2->frame);
}

class Test_HourglassLin : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_LinController* mockLinController{reinterpret_cast<SilKit_LinController*>(uintptr_t(0x12345678))};

    Test_HourglassLin()
    {
        using testing::_;
        ON_CALL(capi, SilKit_LinController_Create(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockLinController), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassLin, SilKit_LinController_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string name = "LinController1";
    std::string network = "LinNetwork1";

    EXPECT_CALL(capi, SilKit_LinController_Create(testing::_, participant, testing::StrEq(name.c_str()),
                                                  testing::StrEq(network.c_str())))
        .Times(1);
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(participant, name,
                                                                                                  network);
}

TEST_F(Test_HourglassLin, SilKit_LinController_Init)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    LinControllerConfig controllerConfig{};
    controllerConfig.baudRate = 1234;
    controllerConfig.controllerMode = LinControllerMode::Master;
    controllerConfig.frameResponses = {{}};

    EXPECT_CALL(capi, SilKit_LinController_Init(mockLinController, LinControllerConfigMatcher(controllerConfig)))
        .Times(1);
    LinController.Init(controllerConfig);
}

TEST_F(Test_HourglassLin, SilKit_LinController_InitDynamic)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    SilKit::Experimental::Services::Lin::LinControllerDynamicConfig controllerConfig{};
    controllerConfig.baudRate = 1234;
    controllerConfig.controllerMode = LinControllerMode::Master;

    EXPECT_CALL(capi, SilKit_Experimental_LinController_InitDynamic(
                          mockLinController, LinControllerDynamicConfigMatcher(controllerConfig)))
        .Times(1);
    LinController.ExperimentalInitDynamic(controllerConfig);
}

TEST_F(Test_HourglassLin, SilKit_LinController_Status)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_Status(mockLinController, testing::_)).Times(1);
    LinController.Status();
}

TEST_F(Test_HourglassLin, SilKit_LinController_SendFrame)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    LinFrame frame{};
    frame.checksumModel = LinChecksumModel::Enhanced;
    frame.id = 123;
    frame.dataLength = 3;
    frame.data = {1, 2, 3, 0, 0, 0, 0, 0};

    EXPECT_CALL(capi, SilKit_LinController_SendFrame(mockLinController, LinFrameMatcher(frame),
                                                     SilKit_LinFrameResponseType_MasterResponse))
        .Times(1);
    LinController.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

TEST_F(Test_HourglassLin, SilKit_LinController_SendFrameHeader)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_SendFrameHeader(mockLinController, 3)).Times(1);
    LinController.SendFrameHeader(3);
}

TEST_F(Test_HourglassLin, SilKit_LinController_UpdateTxBuffer)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    LinFrame frame{};
    frame.checksumModel = LinChecksumModel::Enhanced;
    frame.id = 123;
    frame.dataLength = 3;
    frame.data = {1, 2, 3, 0, 0, 0, 0, 0};

    EXPECT_CALL(capi, SilKit_LinController_UpdateTxBuffer(mockLinController, LinFrameMatcher(frame))).Times(1);
    LinController.UpdateTxBuffer(frame);
}

TEST_F(Test_HourglassLin, SilKit_LinController_SetFrameResponse)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    LinFrame frame{};
    frame.checksumModel = LinChecksumModel::Enhanced;
    frame.id = 123;
    frame.dataLength = 3;
    frame.data = {1, 2, 3, 0, 0, 0, 0, 0};

    LinFrameResponse frameResponse{};
    frameResponse.frame = frame;
    frameResponse.responseMode = LinFrameResponseMode::Rx;

    EXPECT_CALL(capi, SilKit_LinController_SetFrameResponse(mockLinController, LinFrameResponseMatcher(frameResponse)))
        .Times(1);
    LinController.SetFrameResponse(frameResponse);
}

TEST_F(Test_HourglassLin, SilKit_LinController_GoToSleep)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_GoToSleep(mockLinController)).Times(1);
    LinController.GoToSleep();
}

TEST_F(Test_HourglassLin, SilKit_LinController_GoToSleepInternal)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_GoToSleepInternal(mockLinController)).Times(1);
    LinController.GoToSleepInternal();
}

TEST_F(Test_HourglassLin, SilKit_LinController_Wakeup)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_Wakeup(mockLinController)).Times(1);
    LinController.Wakeup();
}

TEST_F(Test_HourglassLin, SilKit_LinController_WakeupInternal)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_WakeupInternal(mockLinController)).Times(1);
    LinController.WakeupInternal();
}

TEST_F(Test_HourglassLin, SilKit_Experimental_LinController_GetSlaveConfiguration)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_Experimental_LinController_GetSlaveConfiguration(mockLinController, testing::_)).Times(1);
    LinController.WakeupInternal();
    SilKit::Experimental::Services::Lin::GetSlaveConfiguration(&LinController);
}

TEST_F(Test_HourglassLin, SilKit_LinController_AddFrameStatusHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_AddFrameStatusHandler(mockLinController, testing::_, testing::_, testing::_))
        .Times(1);
    LinController.AddFrameStatusHandler(nullptr);
}

TEST_F(Test_HourglassLin, SilKit_LinController_RemoveFrameStatusHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_RemoveFrameStatusHandler(mockLinController, testing::_)).Times(1);
    LinController.RemoveFrameStatusHandler({});
}

TEST_F(Test_HourglassLin, SilKit_LinController_AddGoToSleepHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_AddGoToSleepHandler(mockLinController, testing::_, testing::_, testing::_))
        .Times(1);
    LinController.AddGoToSleepHandler(nullptr);
}

TEST_F(Test_HourglassLin, SilKit_LinController_RemoveGoToSleepHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_RemoveGoToSleepHandler(mockLinController, testing::_)).Times(1);
    LinController.RemoveGoToSleepHandler({});
}

TEST_F(Test_HourglassLin, SilKit_LinController_AddWakeupHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_AddWakeupHandler(mockLinController, testing::_, testing::_, testing::_))
        .Times(1);
    LinController.AddWakeupHandler(nullptr);
}

TEST_F(Test_HourglassLin, SilKit_LinController_RemoveWakeupHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_LinController_RemoveWakeupHandler(mockLinController, testing::_)).Times(1);
    LinController.RemoveWakeupHandler({});
}

TEST_F(Test_HourglassLin, SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi, SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(mockLinController, testing::_,
                                                                                        testing::_, testing::_))
        .Times(1);
    SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandler(&LinController, {});
}

TEST_F(Test_HourglassLin, SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Lin::LinController LinController(
        nullptr, "LinController1", "LinNetwork1");

    EXPECT_CALL(capi,
                SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(mockLinController, testing::_))
        .Times(1);
    SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler(&LinController, {});
}
} //namespace
