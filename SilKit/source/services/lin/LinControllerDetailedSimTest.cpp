// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/lin/string_utils.hpp"
#include "silkit/util/functional.hpp"

#include "LinTestUtils.hpp"

namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using namespace testing;
using namespace SilKit::Core;
using namespace SilKit::Services;
using namespace SilKit::Services::Lin;
using namespace SilKit::Services::Lin::Tests;
using namespace SilKit::Util;

class LinControllerDetailedSimTest : public testing::Test
{

protected:
    LinControllerDetailedSimTest()
        : controller(&participant, cfg, participant.GetTimeProvider())
        , controller2(&participant, cfg, participant.GetTimeProvider())
        , controllerBusSim(&participant, cfg, participant.GetTimeProvider())
    {
        frameStatusHandler = [this](ILinController* ctrl, const LinFrameStatusEvent& frameStatusEvent) {
            callbacks.FrameStatusHandler(ctrl, frameStatusEvent.frame, frameStatusEvent.status);
        };
        goToSleepHandler = [this](ILinController* ctrl, const LinGoToSleepEvent& /*goToSleepEvent*/) {
            callbacks.GoToSleepHandler(ctrl);
        };
        wakeupHandler = [this](ILinController* ctrl, const LinWakeupEvent& /*wakeupEvent*/) {
            callbacks.WakeupHandler(ctrl);
        };
        frameResponseUpdateHandler = [this](ILinController* ctrl,
                                            const LinFrameResponseUpdateEvent& frameResponseUpdateEvent) {
            callbacks.FrameResponseUpdateHandler(ctrl, frameResponseUpdateEvent.senderID,
                                                 frameResponseUpdateEvent.frameResponse);
        };

        controller.SetServiceDescriptor(from_endpointAddress(addr1_proxy));
        controller2.SetServiceDescriptor(from_endpointAddress(addr2_proxy));
        controllerBusSim.SetServiceDescriptor(from_endpointAddress(addr1_vibe));
        controller.SetDetailedBehavior(from_endpointAddress(addr1_vibe));
        controller2.SetDetailedBehavior(from_endpointAddress(addr1_vibe));
    }

protected:
    const EndpointAddress addr1_vibe{4, 5};
    const EndpointAddress addr1_proxy{7, 5};
    const EndpointAddress addr2_proxy{4, 9};

    SilKit::Config::LinController cfg;
    LinMockParticipant participant;
    LinController controller;
    LinController controller2;
    LinController controllerBusSim;
    Callbacks callbacks;
    LinController::FrameStatusHandler frameStatusHandler;
    LinController::GoToSleepHandler goToSleepHandler;
    LinController::WakeupHandler wakeupHandler;
    LinController::FrameResponseUpdateHandler frameResponseUpdateHandler;
};

TEST_F(LinControllerDetailedSimTest, send_frame)
{
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    expectedMsg.responseType = LinFrameResponseType::SlaveResponse;

    EXPECT_CALL(participant, SendMsg(&controller, expectedMsg)).Times(1);

    controller.SendFrame(expectedMsg.frame, expectedMsg.responseType);
}

TEST_F(LinControllerDetailedSimTest, send_frame_header)
{
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameHeaderRequest expectedMsg;
    expectedMsg.id = 13;

    EXPECT_CALL(participant, SendMsg(&controller, expectedMsg)).Times(1);

    controller.SendFrameHeader(expectedMsg.id);
}

TEST_F(LinControllerDetailedSimTest, call_frame_status_handler)
{
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    // Receive LinTransmission
    LinFrame rxFrame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});

    // Expect LIN_RX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, rxFrame, LinFrameStatus::LIN_RX_OK)).Times(1);
    LinTransmission transmission;
    transmission.frame = rxFrame;
    transmission.status = LinFrameStatus::LIN_RX_OK;
    controller.ReceiveSilKitMessage(&controllerBusSim, transmission);
}

TEST_F(LinControllerDetailedSimTest, set_frame_response)
{
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    

    LinFrameResponse response;
    response.frame = MakeFrame(19, LinChecksumModel::Enhanced);
    response.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponseUpdate expectedMsg;
    expectedMsg.frameResponses.push_back(response);

    EXPECT_CALL(participant, SendMsg(&controller, expectedMsg)).Times(1);

    controller.SetFrameResponse(response.frame, response.responseMode);
}

TEST_F(LinControllerDetailedSimTest, set_frame_responses)
{
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);


    LinFrameResponse response1;
    response1.frame = MakeFrame(19, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponse response2;
    response2.frame = MakeFrame(3, LinChecksumModel::Classic, 4, std::array<uint8_t, 8>{1, 2, 3, 4, 1, 2, 3, 4});
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    std::vector<LinFrameResponse> responses;
    responses.push_back(response1);
    responses.push_back(response2);


    LinFrameResponseUpdate expectedMsg;
    expectedMsg.frameResponses = responses;

    EXPECT_CALL(participant, SendMsg(&controller, expectedMsg)).Times(1);

    controller.SetFrameResponses(responses);
}

TEST_F(LinControllerDetailedSimTest, trigger_frame_response_update_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    controller.AddFrameResponseUpdateHandler(frameResponseUpdateHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    LinFrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.push_back(response1);
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response2))
        .Times(1);

    controller.ReceiveSilKitMessage(&controller2, responseUpdate);
}

TEST_F(LinControllerDetailedSimTest, trigger_frame_response_update_handler_for_slave_config)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    controller.AddFrameResponseUpdateHandler(frameResponseUpdateHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    LinControllerConfig slaveCfg = MakeControllerConfig(LinControllerMode::Slave);
    slaveCfg.frameResponses.push_back(response1);
    slaveCfg.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response2))
        .Times(1);

    controller.ReceiveSilKitMessage(&controller2, slaveCfg);
}

TEST_F(LinControllerDetailedSimTest, go_to_sleep)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);


    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = GoToSleepFrame();
    expectedMsg.responseType = LinFrameResponseType::MasterResponse;

    EXPECT_CALL(participant, SendMsg(&controller, expectedMsg))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::SleepPending)))
        .Times(1);

    controller.GoToSleep();
}

TEST_F(LinControllerDetailedSimTest, go_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinSendFrameRequest&>()))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Sleep)))
        .Times(1);

    controller.GoToSleepInternal();
}

TEST_F(LinControllerDetailedSimTest, call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    controller.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&controller)).Times(1);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    controller.ReceiveSilKitMessage(&controllerBusSim, goToSleep);
}

TEST_F(LinControllerDetailedSimTest, not_call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    controller.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&controller)).Times(0);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1;
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    controller.ReceiveSilKitMessage(&controllerBusSim, goToSleep);
}

TEST_F(LinControllerDetailedSimTest, wake_up)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinWakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    controller.Wakeup();
}

TEST_F(LinControllerDetailedSimTest, wake_up_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);


    EXPECT_CALL(participant, SendMsg(&controller, A<const LinWakeupPulse&>()))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    controller.WakeupInternal();
}


TEST_F(LinControllerDetailedSimTest, call_wakeup_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    controller.AddWakeupHandler(wakeupHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&controller)).Times(1);

    LinWakeupPulse wakeupPulse;

    controller.ReceiveSilKitMessage(&controllerBusSim, wakeupPulse);
}

// No initialization causes exception
TEST_F(LinControllerDetailedSimTest, go_to_sleep_uninitialized)
{
    EXPECT_THROW(controller.GoToSleep(), SilKit::StateError);
}

TEST_F(LinControllerDetailedSimTest, go_to_sleep_internal_uninitialized)
{
    EXPECT_THROW(controller.GoToSleepInternal(), SilKit::StateError);
}

TEST_F(LinControllerDetailedSimTest, wake_up_uninitialized)
{
    EXPECT_THROW(controller.Wakeup(), SilKit::StateError);
}

TEST_F(LinControllerDetailedSimTest, wake_up_internal_uninitialized)
{
    EXPECT_THROW(controller.WakeupInternal(), SilKit::StateError);
}

} // namespace
