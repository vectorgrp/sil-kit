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

#include "LinController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/lin/string_utils.hpp"
#include "functional.hpp"

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
        : master(&participant, cfg, participant.GetTimeProvider())
        , slave1(&participant, cfg, participant.GetTimeProvider())
        , slave2(&participant, cfg, participant.GetTimeProvider())
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
        slaveConfigurationHandler = [this](ILinController* ctrl,
                   const SilKit::Experimental::Services::Lin::LinSlaveConfigurationEvent& /*slaveConfigurationEvent*/) {
            callbacks.LinSlaveConfigurationHandler(ctrl);
        };
        
        master.SetServiceDescriptor(addr1);

        ON_CALL(participant.mockTimeProvider, Now())
            .WillByDefault(testing::Return(35s));

        slave1.SetServiceDescriptor(addr2);
        slave2.SetServiceDescriptor(addr3);
    }


protected:
    const ServiceDescriptor addr1{"p1", "n1", "c1", 5};
    const ServiceDescriptor addr2{"p2", "n1", "c1", 8};
    const ServiceDescriptor addr3{"p3", "n1", "c1", 13};

    SilKit::Config::LinController cfg;
    LinMockParticipant participant;
    LinController master;
    LinController slave1;
    LinController slave2;
    Callbacks callbacks;
    LinController::FrameStatusHandler frameStatusHandler;
    LinController::GoToSleepHandler goToSleepHandler;
    LinController::WakeupHandler wakeupHandler;
    SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler slaveConfigurationHandler;
    SilKit::Tests::MockTraceSink traceSink;
};


TEST_F(LinControllerTrivialSimTest, throw_on_inactive_init)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Inactive);
    EXPECT_THROW(master.Init(config), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, throw_on_duplicate_init)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    EXPECT_THROW(master.Init(config), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_configured_master_response)
{
    
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrameResponse response;
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    config.frameResponses.push_back(response);
    master.Init(config);

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    master.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_unconfigured_master_response)
{
    
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Master reconfiguration ok
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    master.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_response_via_slave_config)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    
    // Send Frame
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_response_via_slave_response_update)
{
    // Init Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);

    // Let master know about slave
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // LinFrameResponseUpdate from Slave
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    LinFrameResponseUpdate responsesUpdate;
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    responsesUpdate.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, responsesUpdate);

    // Send Frame
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_slave_response_undefined_datalength)
{
    // Init Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);

    // Let master know about slave
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // LinFrameResponseUpdate from Slave
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    LinFrameResponseUpdate responsesUpdate;
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    responsesUpdate.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, responsesUpdate);

    // Send Frame
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_unconfigured_slave_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame frame = MakeFrame(17);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), LinFrameStatus::LIN_RX_NO_RESPONSE))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);

    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}


TEST_F(LinControllerTrivialSimTest, send_frame_with_multiple_slave_responses)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // Configure Slave 2
    slaveConfig.frameResponses[0].frame = MakeFrame(17, LinChecksumModel::Classic, 2, {0,1,0,1,0,1,0,1});
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave2, slaveConfig);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(LinFrameStatus::LIN_RX_ERROR, 35s))).Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), LinFrameStatus::LIN_RX_ERROR)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_master_and_slave_responses)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse slaveResponse;
    slaveResponse.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    slaveResponse.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(slaveResponse);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // Configure Master
    LinControllerConfig masterConfig = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(masterConfig);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Dupicate TX config causes TX/RX error with SendFrame
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(LinFrameStatus::LIN_RX_ERROR, 35s)))
        .Times(1); // Outgoing LIN_RX_ERROR
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), LinFrameStatus::LIN_TX_ERROR))
        .Times(1); // Ack with LIN_TX_ERROR
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);

    LinFrame masterFrame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    master.SendFrame(masterFrame, LinFrameResponseType::MasterResponse);
}


TEST_F(LinControllerTrivialSimTest, send_frame_slave_to_slave_receive_tx)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse slaveResponse;
    slaveResponse.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    slaveResponse.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(slaveResponse);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // Init Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Send SlaveToSlave
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, slaveResponse.frame, LinFrameStatus::LIN_TX_OK));
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
    master.SendFrame(slaveResponse.frame, LinFrameResponseType::SlaveToSlave);
}


TEST_F(LinControllerTrivialSimTest, send_frame_header_with_master_and_slave_responses)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse slaveResponse;
    slaveResponse.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    slaveResponse.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(slaveResponse);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    // Configure Master
    auto masterConfig = MakeControllerConfig(LinControllerMode::Master);
    
    masterConfig.frameResponses.push_back(slaveResponse); // Duplicate TxUnconditional
    master.Init(masterConfig);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Dupicate TX config causes TX/RX error with SendFrameHeader
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(LinFrameStatus::LIN_RX_ERROR, 35s)))
        .Times(1); // Outgoing LIN_RX_ERROR
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), LinFrameStatus::LIN_TX_ERROR))
        .Times(1); // Ack with LIN_TX_ERROR
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
    master.SendFrameHeader(17);
}

TEST_F(LinControllerTrivialSimTest, send_frame_with_one_slave_sleeping)
{
    // Configure Slave 1
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    LinControllerStatusUpdate slaveStatus;
    slaveStatus.status = LinControllerStatus::Sleep;
    master.ReceiveMsg(&slave1, slaveStatus);

    // Configure Master
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);
    
    // Send Frame
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE, 35s))).Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, AFrameWithId(17), LinFrameStatus::LIN_RX_NO_RESPONSE)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_with_unconfigured_response)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame frame = MakeFrame(17);
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(LinFrameStatus::LIN_RX_NO_RESPONSE, 35s))).Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, frame, LinFrameStatus::LIN_RX_NO_RESPONSE)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2); // 1x LinTransmission, 1x local ack
    master.SendFrameHeader(frame.id);

    // Slave without RX receives transmission with LinFrameStatus::LIN_RX_NO_RESPONSE
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    EXPECT_CALL(participant, SendMsg(&slave1, A<const WireLinControllerConfig&>())).Times(1);
    slave1.Init(slaveConfig);
    slave1.AddFrameStatusHandler(frameStatusHandler);
    slave1.ReceiveMsg(&master, LinTransmission{35s, frame, LinFrameStatus::LIN_RX_NO_RESPONSE});
}

TEST_F(LinControllerTrivialSimTest, receive_frame_wrong_checksum)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrame configuredFrame = MakeFrame(17, LinChecksumModel::Enhanced); // Configure with Enhanced
    configuredFrame.dataLength = 8;
    configuredFrame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    LinFrameResponse response;
    response.frame = configuredFrame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame receivedFrame = MakeFrame(configuredFrame.id, LinChecksumModel::Classic); // Receive with Classic 
    receivedFrame.dataLength = configuredFrame.dataLength;

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, receivedFrame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    master.ReceiveMsg(&slave1, LinTransmission{35s, receivedFrame, LinFrameStatus::LIN_RX_OK});
}

TEST_F(LinControllerTrivialSimTest, receive_frame_wrong_datalength)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrame configuredFrame = MakeFrame(17, LinChecksumModel::Enhanced); 
    configuredFrame.dataLength = 8; // Configure with 8
    configuredFrame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    LinFrameResponse response;
    response.frame = configuredFrame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame receivedFrame = MakeFrame(configuredFrame.id, LinChecksumModel::Classic);
    receivedFrame.dataLength = 7;  // Receive with 7

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, receivedFrame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    master.ReceiveMsg(&slave1, LinTransmission{35s, receivedFrame, LinFrameStatus::LIN_RX_OK});
}

TEST_F(LinControllerTrivialSimTest, receive_frame_overwrite_checksum_and_datalength)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrame configuredFrame = MakeFrame(17, LinChecksumModel::Unknown, LinDataLengthUnknown); // Configure with Undefined
    LinFrameResponse response;
    response.frame = configuredFrame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame receivedFrame = MakeFrame(configuredFrame.id, LinChecksumModel::Classic); // Receive with Classic
    receivedFrame.dataLength = 4;

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, receivedFrame, LinFrameStatus::LIN_RX_OK)).Times(1); // Receive ok
    master.ReceiveMsg(&slave1, LinTransmission{35s, receivedFrame, LinFrameStatus::LIN_RX_OK});

    // No overwrite of configured response
    EXPECT_EQ(master.GetThisLinNode().responses[receivedFrame.id].frame.checksumModel,
              configuredFrame.checksumModel); 
    EXPECT_EQ(master.GetThisLinNode().responses[receivedFrame.id].frame.dataLength,
              configuredFrame.dataLength);
}


TEST_F(LinControllerTrivialSimTest, send_frame_header_with_master_tx_slave_rx)
{
    // Master Tx on 17 with preinitialized payload
    auto masterConfig = MakeControllerConfig(LinControllerMode::Master);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    frame.dataLength = 8;
    frame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    masterConfig.frameResponses.push_back(response);
    
    master.Init(masterConfig);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Slaves receives the config
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    slave1.ReceiveMsg(&master, ToWire(masterConfig));

    // Slave Rx on 17 
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    response.responseMode = LinFrameResponseMode::Rx;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant, SendMsg(&slave1, A<const WireLinControllerConfig&>())).Times(1);
    slave1.Init(slaveConfig);
    slave1.AddFrameStatusHandler(frameStatusHandler);
    
    // Master receives the config
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, ToWire(slaveConfig));

    // Master sends the header
    EXPECT_CALL(participant, SendMsg(&master, AHeaderRequestWithId(frame.id))).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.SendFrameHeader(frame.id);

    // Master response: Expect sending the LinTransmission with RX_OK and FrameStatusUpdate with TX_OK on master
    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(frame, LinFrameStatus::LIN_RX_OK, 35s))).Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, frame, LinFrameStatus::LIN_TX_OK)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&master, LinSendFrameHeaderRequest{35s, frame.id});

    // Slave also receives the LinSendFrameHeaderRequest but generates no LinTransmission
    EXPECT_CALL(participant, SendMsg(&slave1, testing::A<const LinTransmission&>())).Times(0);
    slave1.ReceiveMsg(&master, LinSendFrameHeaderRequest{35s, frame.id});

    // Slave calls FrameStatusHandler with RX on reception
    EXPECT_CALL(callbacks, FrameStatusHandler(&slave1, frame, LinFrameStatus::LIN_RX_OK)).Times(1);
    slave1.ReceiveMsg(&master, LinTransmission{35s, frame, LinFrameStatus::LIN_RX_OK });
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_with_master_rx_slave_tx)
{
    // Master Rx on 17 with preinitialized payload
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);
    frame.dataLength = 8;
    frame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Slaves receives the config
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    slave1.ReceiveMsg(&master, ToWire(config));

    // Slave Tx on 17
    LinControllerConfig slaveConfig = MakeControllerConfig(LinControllerMode::Slave);
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant, SendMsg(&slave1, A<const WireLinControllerConfig&>())).Times(1);
    slave1.Init(slaveConfig);
    slave1.AddFrameStatusHandler(frameStatusHandler);

    // Master receives the config
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, ToWire(slaveConfig));

    // Master sends the header
    EXPECT_CALL(participant, SendMsg(&master, AHeaderRequestWithId(frame.id))).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.SendFrameHeader(frame.id);

    // Slave response: Expect sending the LinTransmission with RX_OK and FrameStatusUpdate with TX_OK on slave
    EXPECT_CALL(participant, SendMsg(&slave1, ATransmissionWith(frame, LinFrameStatus::LIN_RX_OK, 35s))).Times(1);
    EXPECT_CALL(callbacks, FrameStatusHandler(&slave1, frame, LinFrameStatus::LIN_TX_OK)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    slave1.ReceiveMsg(&master, LinSendFrameHeaderRequest{35s, frame.id});

    // Master also receives the LinSendFrameHeaderRequest but generates no LinTransmission
    EXPECT_CALL(participant, SendMsg(&master, testing::A<const LinTransmission&>())).Times(0);
    master.ReceiveMsg(&slave1, LinSendFrameHeaderRequest{35s, frame.id});

    // Master calls FrameStatusHandler with RX on reception
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, frame, LinFrameStatus::LIN_RX_OK)).Times(1);
    master.ReceiveMsg(&slave1, LinTransmission{35s, frame, LinFrameStatus::LIN_RX_OK});
}

TEST_F(LinControllerTrivialSimTest, trigger_slave_callbacks)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4);
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    LinFrame txFrame = MakeFrame(18, LinChecksumModel::Classic, 2, {1,2,0,0,0,0,0,0});
    response.frame = txFrame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    config.frameResponses.push_back(response);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Receive LinTransmission
    LinTransmission transmission;

    // Expect LIN_RX_OK
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, transmission.frame, LinFrameStatus::LIN_RX_OK)).Times(1);
    master.ReceiveMsg(&slave1, transmission);

    // Expect LIN_RX_ERROR due to dataLength mismatch
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 2, {1,2,0,0,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, transmission.frame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    master.ReceiveMsg(&slave1, transmission);

    // Expect LIN_RX_ERROR due to checksumModel mismatch
    transmission.frame = MakeFrame(17, LinChecksumModel::Classic, 4, {1,2,3,4,0,0,0,0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, transmission.frame, LinFrameStatus::LIN_RX_ERROR)).Times(1);
    master.ReceiveMsg(&slave1, transmission);

    // Expect LIN_TX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, txFrame, LinFrameStatus::LIN_TX_OK)).Times(1);
    transmission.frame = txFrame;
    master.ReceiveMsg(&slave1, transmission);

    // Expect no callback with LIN_RX_NO_RESPONSE
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), LinFrameStatus::LIN_RX_NO_RESPONSE)).Times(0);
    transmission.frame.id = 19;
    master.ReceiveMsg(&slave1, transmission);
}

TEST_F(LinControllerTrivialSimTest, trigger_slave_configuration_handler)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));
    master.AddLinSlaveConfigurationHandler(slaveConfigurationHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    LinControllerConfig slaveCfg = MakeControllerConfig(LinControllerMode::Slave);
    slaveCfg.frameResponses.push_back(response1);
    slaveCfg.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, LinSlaveConfigurationHandler(&master)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, ToWire(slaveCfg));
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));

    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(GoToSleepFrame(), LinFrameStatus::LIN_RX_OK))).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Sleep))).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.GoToSleep();
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep_internal)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));

    EXPECT_CALL(participant, SendMsg(&master, ATransmissionWith(GoToSleepFrame(), LinFrameStatus::LIN_RX_OK)))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Sleep)))
        .Times(1);

    master.GoToSleepInternal();
}

TEST_F(LinControllerTrivialSimTest, call_gotosleep_handler)
{
    EXPECT_CALL(participant, SendMsg(&slave1, A<const WireLinControllerConfig&>())).Times(1);
    slave1.Init(MakeControllerConfig(LinControllerMode::Slave));
    slave1.AddGoToSleepHandler(goToSleepHandler);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = LinFrameStatus::LIN_TX_OK;
    EXPECT_CALL(callbacks, GoToSleepHandler(&slave1)).Times(1);
    slave1.ReceiveMsg(&master, goToSleep);
}

TEST_F(LinControllerTrivialSimTest, not_call_gotosleep_handler)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));
    master.AddFrameStatusHandler(frameStatusHandler);
    master.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), _)).Times(0); // No response configured
    EXPECT_CALL(callbacks, GoToSleepHandler(&master)).Times(0);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1; // Invalid GoToSleep-Frame
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    master.ReceiveMsg(&slave1, goToSleep);
}

TEST_F(LinControllerTrivialSimTest, wake_up)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));

    EXPECT_CALL(participant, SendMsg(&master, A<const LinWakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now())
        .Times(1);

    master.Wakeup();
}

TEST_F(LinControllerTrivialSimTest, wake_up_internal)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    EXPECT_CALL(participant, SendMsg(&master, A<const LinWakeupPulse&>()))
        .Times(0);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    master.WakeupInternal();
}


TEST_F(LinControllerTrivialSimTest, call_wakeup_handler)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));
    master.AddFrameStatusHandler(frameStatusHandler);
    master.AddWakeupHandler(wakeupHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&master)).Times(1);

    LinWakeupPulse wakeupPulse;

    master.ReceiveMsg(&slave1, wakeupPulse);
}

TEST_F(LinControllerTrivialSimTest, receive_frame_unknown_checksum)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrame configuredFrame =
        MakeFrame(17, LinChecksumModel::Enhanced, 4); // Configure with Enhanced
    LinFrameResponse response;
    response.frame = configuredFrame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);

    slave1.Init(config);
    slave1.AddFrameStatusHandler(frameStatusHandler);

    LinFrame receivedFrame = MakeFrame(configuredFrame.id, LinChecksumModel::Unknown); // Receive with Unknown
    receivedFrame.dataLength = 4;

    EXPECT_CALL(callbacks, FrameStatusHandler(&slave1, receivedFrame, LinFrameStatus::LIN_RX_OK))
        .Times(1); // Receive ok
    slave1.ReceiveMsg(&master, LinTransmission{35s, receivedFrame, LinFrameStatus::LIN_RX_OK});
}

TEST_F(LinControllerTrivialSimTest, send_frame_master_response_throw_on_unitialized_datalength)
{
    master.Init(MakeControllerConfig(LinControllerMode::Master));

    // SendFrame configures TX on Master
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameHeaderRequest&>())).Times(1);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Classic, LinDataLengthUnknown);
    master.SendFrame(frame, LinFrameResponseType::MasterResponse);

    EXPECT_THROW(master.ReceiveMsg(&master, LinSendFrameHeaderRequest{35s, frame.id}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep_throw_uninitialized)
{
    EXPECT_THROW(master.GoToSleep(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, go_to_sleep_internal_throw_uninitialized)
{
    EXPECT_THROW(master.GoToSleepInternal(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, wake_up_throw_uninitialized)
{
    EXPECT_THROW(master.Wakeup(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, send_frame_throw_unitialized)
{
    EXPECT_THROW(master.SendFrame({}, LinFrameResponseType::MasterResponse), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, send_frame_header_throw_unitialized)
{
    EXPECT_THROW(master.SendFrameHeader({}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, update_tx_buffer_throw_unitialized)
{
    EXPECT_THROW(master.UpdateTxBuffer({}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, wake_up_internal_throw_uninitialized)
{
    EXPECT_THROW(master.WakeupInternal(), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, set_frame_response_throw_uninitialized)
{
    EXPECT_THROW(master.SetFrameResponse({}), SilKit::StateError);
}

TEST_F(LinControllerTrivialSimTest, add_remove_handler)
{
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    LinFrameResponse response;
    response.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4);
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses.push_back(response);
    master.Init(config);

    const int numHandlers = 10;
    std::vector<SilKit::Services::HandlerId> handlerIds;
    for (int i = 0; i < numHandlers; i++)
    {
        handlerIds.push_back(master.AddFrameStatusHandler(frameStatusHandler));
    }

    // Receive LinTransmission
    LinTransmission transmission;

    // Expect LIN_RX_OK
    transmission.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 0, 0, 0, 0});
    transmission.status = LinFrameStatus::LIN_RX_OK;
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, transmission.frame, LinFrameStatus::LIN_RX_OK))
        .Times(numHandlers);

    master.ReceiveMsg(&slave1, transmission);

    for (auto&& handlerId : handlerIds)
    {
        master.RemoveFrameStatusHandler(handlerId);
    }

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, transmission.frame, LinFrameStatus::LIN_RX_OK))
        .Times(0);
    master.ReceiveMsg(&slave1, transmission);
}

////////////
// Tracing
////////////

TEST_F(LinControllerTrivialSimTest, DISABLED_send_with_tracing)
{
    

    const auto now = 0x0815ns;
    ON_CALL(participant.mockTimeProvider, Now())
        .WillByDefault(testing::Return(now));

    master.AddSink(&traceSink, SilKit::Config::NetworkType::LIN);

    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced);

    EXPECT_CALL(participant.mockTimeProvider, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(SilKit::Services::TransmitDirection::TX, master.GetServiceDescriptor(), now, frame))
        .Times(1);

    master.SendFrame(frame, LinFrameResponseType::MasterResponse);
}

} // anonymous namespace
