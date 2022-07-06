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
#include "MockTraceSink.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;
using namespace SilKit::Core;
using namespace SilKit::Services;
using namespace SilKit::Services::Lin;
using namespace SilKit::Services::Lin::Tests;
using namespace SilKit::Util;

class LinControllerTrivialSimTest : public testing::Test
{
protected:
    LinControllerTrivialSimTest()
        : controller(&participant, cfg, participant.GetTimeProvider())
        , controller2(&participant, cfg, participant.GetTimeProvider())
        , controller3(&participant, cfg, participant.GetTimeProvider())
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
        
        controller.SetServiceDescriptor(from_endpointAddress(addr1));

        ON_CALL(participant.mockTimeProvider.mockTime, Now())
            .WillByDefault(testing::Return(35s));

        controller2.SetServiceDescriptor(from_endpointAddress(addr2));
        controller3.SetServiceDescriptor(from_endpointAddress(addr3));
    }


protected:
    const EndpointAddress addr1{4, 5};
    const EndpointAddress addr2{5, 8};
    const EndpointAddress addr3{6, 13};

    SilKit::Config::LinController cfg;
    LinMockParticipant participant;
    LinController controller;
    LinController controller2;
    LinController controller3;
    Callbacks callbacks;
    LinController::FrameStatusHandler frameStatusHandler;
    LinController::GoToSleepHandler goToSleepHandler;
    LinController::WakeupHandler wakeupHandler;
    LinController::FrameResponseUpdateHandler frameResponseUpdateHandler;
    SilKit::Tests::MockTraceSink traceSink;
};


TEST_F(LinControllerTrivialSimTest, send_frame_with_master_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()))
        .Times(1);
    controller.Init(config);

    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    controller.AddFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(frame, LinFrameStatus::LIN_RX_OK, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, LinFrameStatus::LIN_TX_OK))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_without_configured_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()))
        .Times(1);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    LinFrame frame = MakeFrame(17);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), LinFrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_response)
{
    // Configure Slave 1
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveSilKitMessage(&controller2, slaveConfig);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()))
        .Times(1);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(response.frame, LinFrameStatus::LIN_RX_OK, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, response.frame, LinFrameStatus::LIN_RX_OK))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_multiple_slave_responses)
{
    // Configure Slave 1
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveSilKitMessage(&controller2, slaveConfig);

    // Configure Slave 2
    slaveConfig.frameResponses[0].frame = MakeFrame(17, LinChecksumModel::Classic, 2, {0,1,0,1,0,1,0,1});

    controller.ReceiveSilKitMessage(&controller3, slaveConfig);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()))
        .Times(1);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(LinFrameStatus::LIN_RX_ERROR, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), LinFrameStatus::LIN_RX_ERROR))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_sleeping)
{
    // Configure Slave 1
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveSilKitMessage(&controller2, slaveConfig);

    LinControllerStatusUpdate slaveStatus;
    slaveStatus.status = LinControllerStatus::Sleep;
    controller.ReceiveSilKitMessage(&controller2, slaveStatus);

    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), LinFrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_response_removed)
{
    // Configure Slave 1
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveSilKitMessage(&controller2, slaveConfig);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>())).Times(1);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(response.frame, LinFrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, response.frame, LinFrameStatus::LIN_RX_OK))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);
    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);

    // Remove Frame Response by marking it as unused
    response.responseMode = LinFrameResponseMode::Unused;
    LinFrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.push_back(response);
    controller.ReceiveSilKitMessage(&controller2, responseUpdate);

    // Send Frame again
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), LinFrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_with_master_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>())).Times(1);
    controller.Init(config);

    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    controller.AddFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(frame, LinFrameStatus::LIN_RX_OK, 35s)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, LinFrameStatus::LIN_TX_OK))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SetFrameResponse(frame, LinFrameResponseMode::TxUnconditional);
    controller.SendFrameHeader(17);
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_remove_master_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>())).Times(1);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    // Set Frame Response and Send Header
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(frame, LinFrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, LinFrameStatus::LIN_TX_OK))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SetFrameResponse(frame, LinFrameResponseMode::TxUnconditional);
    controller.SendFrameHeader(17);

    // Mark Frame as Unused and Send Header Again
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinFrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), LinFrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.SetFrameResponse(frame, LinFrameResponseMode::Unused);
    controller.SendFrameHeader(17);
}

TEST_F(LinControllerTrivialSimTest, trigger_slave_callbacks)
{
    // Configure Slave
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;
    
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4);
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);

    LinFrame txFrame = MakeFrame(18, LinChecksumModel::Classic, 2, {1,2,0,0,0,0,0,0});
    response.frame = txFrame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    config.frameResponses.push_back(response);

    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);

    // Receive LinTransmission
    LinTransmission transmission;

    // Expect LIN_RX_OK
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, LinFrameStatus::LIN_RX_OK)).Times(1);
    controller.ReceiveSilKitMessage(&controller2, transmission);

    // Expect LIN_RX_ERRROR due to dataLength mismatch
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 2, {1,2,0,0,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    controller.ReceiveSilKitMessage(&controller2, transmission);

    // Expect LIN_RX_ERRROR due to checksumModel mismatch
    transmission.frame = MakeFrame(17, LinChecksumModel::Classic, 4, {1,2,3,4,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    controller.ReceiveSilKitMessage(&controller2, transmission);

    // Expect LIN_TX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, txFrame, LinFrameStatus::LIN_TX_OK)).Times(1);
    transmission.frame = txFrame;
    controller.ReceiveSilKitMessage(&controller2, transmission);

    // Expect no callback with LIN_RX_NO_RESPONSE
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), LinFrameStatus::LIN_RX_NO_RESPONSE)).Times(0);
    transmission.frame.id = 19;
    controller.ReceiveSilKitMessage(&controller2, transmission);
}

TEST_F(LinControllerTrivialSimTest, distribute_frame_response_updates)
{
    // Configure Slave
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);

    LinFrameResponseUpdate responseUpdate;
    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    responseUpdate.frameResponses.push_back(response1);
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(participant, SendMsg(&controller, responseUpdate))
        .Times(1);

    controller.SetFrameResponses(responseUpdate.frameResponses);
}

TEST_F(LinControllerTrivialSimTest, trigger_frame_response_update_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    controller.AddFrameResponseUpdateHandler(frameResponseUpdateHandler);

    LinFrameResponseUpdate responseUpdate;
    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    responseUpdate.frameResponses.push_back(response1);
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, to_string(controller2.GetServiceDescriptor()), response2))
        .Times(1);

    controller.ReceiveSilKitMessage(&controller2, responseUpdate);
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(GoToSleepFrame(), LinFrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Sleep)))
        .Times(1);

    controller.GoToSleep();
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(participant, SendMsg(&controller, ATransmissionWith(GoToSleepFrame(), LinFrameStatus::LIN_RX_OK)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Sleep)))
        .Times(1);

    controller.GoToSleepInternal();
}

TEST_F(LinControllerTrivialSimTest, call_gotosleep_handler)
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

    controller.ReceiveSilKitMessage(&controller2, goToSleep);
}

TEST_F(LinControllerTrivialSimTest, not_call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    controller.Init(config);
    controller.AddFrameStatusHandler(frameStatusHandler);
    controller.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const LinFrame&>(), _)).Times(0); // No reponse configured
    EXPECT_CALL(callbacks, GoToSleepHandler(&controller)).Times(0);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1; // Invalid GoToSleep-Frame
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    controller.ReceiveSilKitMessage(&controller2, goToSleep);
}

TEST_F(LinControllerTrivialSimTest, wake_up)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    controller.Init(MakeControllerConfig(LinControllerMode::Master));

    EXPECT_CALL(participant, SendMsg(&controller, A<const LinWakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&controller, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);

    controller.Wakeup();
}

TEST_F(LinControllerTrivialSimTest, wake_up_internal)
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


TEST_F(LinControllerTrivialSimTest, call_wakeup_handler)
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

    controller.ReceiveSilKitMessage(&controller2, wakeupPulse);
}

// No initialization causes exception
TEST_F(LinControllerTrivialSimTest, go_to_sleep_uninitialized)
{
    EXPECT_THROW(controller.GoToSleep(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep_internal_uninitialized)
{
    EXPECT_THROW(controller.GoToSleepInternal(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, wake_up_uninitialized)
{
    EXPECT_THROW(controller.Wakeup(), SilKit::StateError);
}


TEST_F(LinControllerTrivialSimTest, send_frame_unitialized)
{
    EXPECT_THROW(controller.SendFrame({}, LinFrameResponseType::MasterResponse), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_unitialized)
{
    EXPECT_THROW(controller.SendFrameHeader({}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, set_responses_unitialized)
{
    EXPECT_THROW(controller.SetFrameResponses({}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, wake_up_internal_uninitialized)
{
    EXPECT_THROW(controller.WakeupInternal(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, add_remove_handler)
{
    // Configure Slave
    EXPECT_CALL(participant, SendMsg(&controller, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    LinFrameResponse response;

    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4);
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);

    controller.Init(config);
    const int numHandlers = 10;
    std::vector<SilKit::Services::HandlerId> handlerIds;
    for (int i = 0; i < numHandlers; i++)
    {
        handlerIds.push_back(controller.AddFrameStatusHandler(frameStatusHandler));
    }

    // Receive LinTransmission
    LinTransmission transmission;

    // Expect LIN_RX_OK
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 0, 0, 0, 0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, LinFrameStatus::LIN_RX_OK))
        .Times(numHandlers);

    controller.ReceiveSilKitMessage(&controller2, transmission);

    for (auto&& handlerId : handlerIds)
    {
        controller.RemoveFrameStatusHandler(handlerId);
    }

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, LinFrameStatus::LIN_RX_OK))
        .Times(0);
    controller.ReceiveSilKitMessage(&controller2, transmission);
}



////////////
// Tracing
////////////

TEST_F(LinControllerTrivialSimTest, DISABLED_send_with_tracing)
{
    

    const auto now = 0x0815ns;
    ON_CALL(participant.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    controller.AddSink(&traceSink);

    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    controller.Init(config);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(SilKit::Services::TransmitDirection::TX, addr1, now, frame))
        .Times(1);

    controller.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

} // anonymous namespace
