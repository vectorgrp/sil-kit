// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "LinTestUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;
using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::lin;
using namespace ib::sim::lin::test;
using namespace ib::util;

class LinControllerTest : public testing::Test
{
protected:
    LinControllerTest()
        : controller(&comAdapter, comAdapter.GetTimeProvider())
    {
        frameStatusHandler = 
            [this](ILinController* ctrl, const Frame& frame, FrameStatus status, std::chrono::nanoseconds) {
                callbacks.FrameStatusHandler(ctrl, frame, status);
            };
        
        controller.SetEndpointAddress(ibAddr1);
    }


protected:
    const EndpointAddress ibAddr1{4, 5};
    const EndpointAddress ibAddr2{5, 8};
    const EndpointAddress ibAddr3{6, 13};

    LinMockComAdapter comAdapter;
    LinController controller;
    Callbacks callbacks;
    LinController::FrameStatusHandler frameStatusHandler;
};


TEST_F(LinControllerTest, send_frame_with_master_response)
{
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    controller.RegisterFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(frame, FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, FrameStatus::LIN_TX_OK))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::MasterResponse);
}

TEST_F(LinControllerTest, send_frame_with_master_response_and_timestamp)
{
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);
    auto timestamp = 70ms;

    controller.RegisterFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(frame, FrameStatus::LIN_RX_OK, timestamp)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, FrameStatus::LIN_TX_OK))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::MasterResponse, timestamp);
}

TEST_F(LinControllerTest, send_frame_without_configured_response)
{
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);

    Frame frame = MakeFrame(17);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(FrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), FrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    controller.SendFrame(frame, FrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTest, send_frame_with_one_slave_response)
{
    // Configure Slave 1
    ControllerConfig slaveConfig = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = FrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveIbMessage(ibAddr2, slaveConfig);

    // Configure Master
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(response.frame, FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, response.frame, FrameStatus::LIN_RX_OK))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTest, send_frame_with_one_slave_response_and_timestamp)
{
    // Configure Slave 1
    ControllerConfig slaveConfig = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = FrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveIbMessage(ibAddr2, slaveConfig);

    // Configure Master
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);
    auto timestamp = 17s;

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(response.frame, FrameStatus::LIN_RX_OK, timestamp)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, response.frame, FrameStatus::LIN_RX_OK))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse, timestamp);
}


TEST_F(LinControllerTest, send_frame_with_multiple_slave_responses)
{
    // Configure Slave 1
    ControllerConfig slaveConfig = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = FrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveIbMessage(ibAddr2, slaveConfig);

    // Configure Slave 2
    slaveConfig.frameResponses[0].frame = MakeFrame(17, ChecksumModel::Classic, 2, {0,1,0,1,0,1,0,1});

    controller.ReceiveIbMessage(ibAddr3, slaveConfig);

    // Configure Master
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(FrameStatus::LIN_RX_ERROR)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), FrameStatus::LIN_RX_ERROR))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTest, send_frame_with_one_slave_sleeping)
{
    // Configure Slave 1
    ControllerConfig slaveConfig = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = FrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveIbMessage(ibAddr2, slaveConfig);

    ControllerStatusUpdate slaveStatus;
    slaveStatus.status = ControllerStatus::Sleep;
    controller.ReceiveIbMessage(ibAddr2, slaveStatus);

    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(FrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), FrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTest, send_frame_with_one_slave_response_removed)
{
    // Configure Slave 1
    ControllerConfig slaveConfig = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = FrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);

    controller.ReceiveIbMessage(ibAddr2, slaveConfig);

    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(response.frame, FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, response.frame, FrameStatus::LIN_RX_OK))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse);

    // Remove Frame Response by marking it as unused
    response.responseMode = FrameResponseMode::Unused;
    FrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.push_back(response);
    controller.ReceiveIbMessage(ibAddr2, responseUpdate);

    // Send Frame again
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(FrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), FrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);

    controller.SendFrame(frame, FrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTest, send_frame_header_with_master_response)
{
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);

    controller.RegisterFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(frame, FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, FrameStatus::LIN_TX_OK))
        .Times(1);

    controller.SetFrameResponse(frame, FrameResponseMode::TxUnconditional);
    controller.SendFrameHeader(17);
}

TEST_F(LinControllerTest, send_frame_header_with_master_response_and_timestamp)
{
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);
    auto timestamp = 45ns;

    controller.RegisterFrameStatusHandler(frameStatusHandler);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(frame, FrameStatus::LIN_RX_OK, timestamp)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, FrameStatus::LIN_TX_OK))
        .Times(1);

    controller.SetFrameResponse(frame, FrameResponseMode::TxUnconditional);
    controller.SendFrameHeader(17, timestamp);
}

TEST_F(LinControllerTest, send_frame_header_remove_master_response)
{
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);

    // Set Frame Response and Send Header
    Frame frame = MakeFrame(17, ChecksumModel::Enhanced);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(frame, FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, frame, FrameStatus::LIN_TX_OK))
        .Times(1);

    controller.SetFrameResponse(frame, FrameResponseMode::TxUnconditional);
    controller.SendFrameHeader(17);

    // Mark Frame as Unused and Send Header Again
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const FrameResponseUpdate&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(FrameStatus::LIN_RX_NO_RESPONSE)))
        .Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, AFrameWithId(17), FrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    controller.SetFrameResponse(frame, FrameResponseMode::Unused);
    controller.SendFrameHeader(17);
}

TEST_F(LinControllerTest, trigger_slave_callbacks)
{
    // Configure Slave
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    FrameResponse response;
    
    response.frame = MakeFrame(17, ChecksumModel::Enhanced, 4);
    response.responseMode = FrameResponseMode::Rx;
    config.frameResponses.push_back(response);

    Frame txFrame = MakeFrame(18, ChecksumModel::Classic, 2, {1,2,0,0,0,0,0,0});
    response.frame = txFrame;
    response.responseMode = FrameResponseMode::TxUnconditional;
    config.frameResponses.push_back(response);

    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);

    // Receive Transmission
    Transmission transmission;

    // Expect LIN_RX_OK
    transmission.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});
    transmission.status = FrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, FrameStatus::LIN_RX_OK)).Times(1);
    controller.ReceiveIbMessage(ibAddr2, transmission);

    // Expect LIN_RX_ERRROR due to dataLength mismatch
    transmission.frame = MakeFrame(17, ChecksumModel::Enhanced, 2, {1,2,0,0,0,0,0,0});
    transmission.status = FrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, FrameStatus::LIN_RX_ERROR)).Times(1);
    controller.ReceiveIbMessage(ibAddr2, transmission);

    // Expect LIN_RX_ERRROR due to checksumModel mismatch
    transmission.frame = MakeFrame(17, ChecksumModel::Classic, 4, {1,2,3,4,0,0,0,0});
    transmission.status = FrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, transmission.frame, FrameStatus::LIN_RX_ERROR)).Times(1);
    controller.ReceiveIbMessage(ibAddr2, transmission);

    // Expect LIN_TX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, txFrame, FrameStatus::LIN_TX_OK)).Times(1);
    transmission.frame = txFrame;
    controller.ReceiveIbMessage(ibAddr2, transmission);

    // Expect no call at all
    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), A<FrameStatus>())).Times(0);
    transmission.frame.id = 19;
    controller.ReceiveIbMessage(ibAddr2, transmission);
}

TEST_F(LinControllerTest, distribute_frame_response_updates)
{
    // Configure Slave
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    controller.Init(config);

    FrameResponseUpdate responseUpdate;
    FrameResponse response1;
    response1.frame = MakeFrame(17, ChecksumModel::Enhanced);
    response1.responseMode = FrameResponseMode::Rx;
    responseUpdate.frameResponses.push_back(response1);
    FrameResponse response2;
    response2.frame = MakeFrame(19, ChecksumModel::Classic);
    response2.responseMode = FrameResponseMode::TxUnconditional;
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, responseUpdate))
        .Times(1);

    controller.SetFrameResponses(responseUpdate.frameResponses);
}

TEST_F(LinControllerTest, trigger_frame_response_update_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    controller.RegisterFrameResponseUpdateHandler(bind_method(&callbacks, &Callbacks::FrameResponseUpdateHandler));

    FrameResponseUpdate responseUpdate;
    FrameResponse response1;
    response1.frame = MakeFrame(17, ChecksumModel::Enhanced);
    response1.responseMode = FrameResponseMode::Rx;
    responseUpdate.frameResponses.push_back(response1);
    FrameResponse response2;
    response2.frame = MakeFrame(19, ChecksumModel::Classic);
    response2.responseMode = FrameResponseMode::TxUnconditional;
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, ibAddr2, response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&controller, ibAddr2, response2))
        .Times(1);

    controller.ReceiveIbMessage(ibAddr2, responseUpdate);
}

TEST_F(LinControllerTest, go_to_sleep)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(GoToSleepFrame(), FrameStatus::LIN_RX_OK)))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, AControllerStatusUpdateWith(ControllerStatus::Sleep)))
        .Times(1);

    controller.GoToSleep();
}

TEST_F(LinControllerTest, go_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);

    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, ATransmissionWith(GoToSleepFrame(), FrameStatus::LIN_RX_OK)))
        .Times(0);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, AControllerStatusUpdateWith(ControllerStatus::Sleep)))
        .Times(1);

    controller.GoToSleepInternal();
}

TEST_F(LinControllerTest, call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    controller.RegisterGoToSleepHandler(bind_method(&callbacks, &Callbacks::GoToSleepHandler));

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, GoToSleepHandler(&controller)).Times(1);

    Transmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = FrameStatus::LIN_RX_OK;

    controller.ReceiveIbMessage(ibAddr2, goToSleep);
}

TEST_F(LinControllerTest, not_call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    controller.RegisterGoToSleepHandler(bind_method(&callbacks, &Callbacks::GoToSleepHandler));

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, GoToSleepHandler(&controller)).Times(0);

    Transmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1;
    goToSleep.status = FrameStatus::LIN_RX_OK;

    controller.ReceiveIbMessage(ibAddr2, goToSleep);
}

TEST_F(LinControllerTest, wake_up)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);


    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const WakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, AControllerStatusUpdateWith(ControllerStatus::Operational)))
        .Times(1);

    controller.Wakeup();
}

TEST_F(LinControllerTest, wake_up_internal)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    controller.Init(config);


    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const WakeupPulse&>()))
        .Times(0);
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, AControllerStatusUpdateWith(ControllerStatus::Operational)))
        .Times(1);

    controller.WakeupInternal();
}


TEST_F(LinControllerTest, call_wakeup_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(ibAddr1, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    controller.Init(config);
    controller.RegisterFrameStatusHandler(frameStatusHandler);
    controller.RegisterWakeupHandler(bind_method(&callbacks, &Callbacks::WakeupHandler));

    EXPECT_CALL(callbacks, FrameStatusHandler(&controller, A<const Frame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&controller)).Times(1);

    WakeupPulse wakeupPulse;

    controller.ReceiveIbMessage(ibAddr2, wakeupPulse);
}

} // anonymous namespace
