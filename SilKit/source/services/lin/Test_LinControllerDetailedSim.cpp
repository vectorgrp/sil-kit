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

namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using namespace testing;
using namespace SilKit::Core;
using namespace SilKit::Services;
using namespace SilKit::Services::Lin;
using namespace SilKit::Services::Lin::Tests;
using namespace SilKit::Util;

class Test_LinControllerDetailedSim : public testing::Test
{
protected:
    Test_LinControllerDetailedSim()
        : master(&participant, cfg, participant.GetTimeProvider())
        , slave1(&participant, cfg, participant.GetTimeProvider())
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
        slaveConfigurationHandler = [this](ILinController* ctrl,
                   const SilKit::Experimental::Services::Lin::LinSlaveConfigurationEvent& /*slaveConfigurationEvent*/) {
            callbacks.LinSlaveConfigurationHandler(ctrl);
        };

        master.SetServiceDescriptor(addr1_proxy);
        slave1.SetServiceDescriptor(addr2_proxy);
        controllerBusSim.SetServiceDescriptor(addr1_netsim);
        master.SetDetailedBehavior(addr1_netsim);
        slave1.SetDetailedBehavior(addr1_netsim);
    }

protected:
    const ServiceDescriptor addr1_netsim{"P1", "N1", "C1",5};
    const ServiceDescriptor addr1_proxy{"P2", "N1", "C1", 5};
    const ServiceDescriptor addr2_proxy{"P1", "N1", "C2", 9};

    SilKit::Config::LinController cfg;
    LinMockParticipant participant;
    LinController master;
    LinController slave1;
    LinController controllerBusSim;
    Callbacks callbacks;
    LinController::FrameStatusHandler frameStatusHandler;
    LinController::GoToSleepHandler goToSleepHandler;
    LinController::WakeupHandler wakeupHandler;
    SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler slaveConfigurationHandler;
};

TEST_F(Test_LinControllerDetailedSim, send_frame_unitialized)
{
    EXPECT_THROW(master.SendFrame({}, LinFrameResponseType::MasterResponse), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, send_frame)
{
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});
    expectedMsg.responseType = LinFrameResponseType::SlaveResponse;

    // Slave has to be configured with Tx on 17
    auto slaveConfig = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    LinFrameResponse response;
    response.frame = expectedMsg.frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    slaveConfig.frameResponses.push_back(response);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveConfig);

    EXPECT_CALL(participant, SendMsg(&master, A<const LinFrameResponseUpdate&>())).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, expectedMsg)).Times(1);
    master.SendFrame(expectedMsg.frame, expectedMsg.responseType);
}

TEST_F(Test_LinControllerDetailedSim, send_frame_without_configured_slave_response)
{
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinFrame frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 5, 6, 7, 8});

    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameRequest&>())).Times(0);
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, frame, LinFrameStatus::LIN_RX_NO_RESPONSE)).Times(2);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(2);
    master.SendFrame(frame, LinFrameResponseType::SlaveResponse);
    master.SendFrame(frame, LinFrameResponseType::SlaveToSlave);
}

TEST_F(Test_LinControllerDetailedSim, send_frame_header_unitialized)
{
    EXPECT_THROW(master.SendFrameHeader({}), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, send_frame_header)
{
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameHeaderRequest expectedMsg;
    expectedMsg.id = 13;

    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, expectedMsg)).Times(1);
    master.SendFrameHeader(expectedMsg.id);
}

TEST_F(Test_LinControllerDetailedSim, call_frame_status_handler)
{
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);

    // Receive LinTransmission
    LinFrame rxFrame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1, 2, 3, 4, 0, 0, 0, 0});

    // Expect LIN_RX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&master, rxFrame, LinFrameStatus::LIN_RX_OK)).Times(1);
    LinTransmission transmission;
    transmission.frame = rxFrame;
    transmission.status = LinFrameStatus::LIN_RX_OK;
    master.ReceiveMsg(&controllerBusSim, transmission);
}

TEST_F(Test_LinControllerDetailedSim, trigger_slave_configuration_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    master.AddLinSlaveConfigurationHandler(slaveConfigurationHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    auto slaveCfg = ToWire(MakeControllerConfig(LinControllerMode::Slave));
    slaveCfg.frameResponses.push_back(response1);
    slaveCfg.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, LinSlaveConfigurationHandler(&master)).Times(1);
    EXPECT_CALL(participant.mockTimeProvider, Now()).Times(1);
    master.ReceiveMsg(&slave1, slaveCfg);
}

TEST_F(Test_LinControllerDetailedSim, go_to_sleep)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = GoToSleepFrame();
    expectedMsg.responseType = LinFrameResponseType::MasterResponse;

    EXPECT_CALL(participant, SendMsg(&master, expectedMsg)).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::SleepPending)))
        .Times(1);

    master.GoToSleep();
}

TEST_F(Test_LinControllerDetailedSim, go_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    EXPECT_CALL(participant, SendMsg(&master, A<const LinSendFrameRequest&>())).Times(0);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Sleep))).Times(1);

    master.GoToSleepInternal();
}

TEST_F(Test_LinControllerDetailedSim, call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);
    master.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&master)).Times(1);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = LinFrameStatus::LIN_TX_OK;

    master.ReceiveMsg(&controllerBusSim, goToSleep);
}

TEST_F(Test_LinControllerDetailedSim, not_call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);
    master.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&master)).Times(0);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1;
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    master.ReceiveMsg(&controllerBusSim, goToSleep);
}

TEST_F(Test_LinControllerDetailedSim, wake_up)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    EXPECT_CALL(participant, SendMsg(&master, A<const LinWakeupPulse&>())).Times(1);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    master.Wakeup();
}

TEST_F(Test_LinControllerDetailedSim, wake_up_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Master);
    master.Init(config);

    EXPECT_CALL(participant, SendMsg(&master, A<const LinWakeupPulse&>())).Times(0);
    EXPECT_CALL(participant, SendMsg(&master, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    master.WakeupInternal();
}

TEST_F(Test_LinControllerDetailedSim, call_wakeup_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    master.Init(config);
    master.AddFrameStatusHandler(frameStatusHandler);
    master.AddWakeupHandler(wakeupHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&master, A<const LinFrame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&master)).Times(1);

    LinWakeupPulse wakeupPulse;

    master.ReceiveMsg(&controllerBusSim, wakeupPulse);
}

// No initialization causes exception
TEST_F(Test_LinControllerDetailedSim, go_to_sleep_uninitialized)
{
    EXPECT_THROW(master.GoToSleep(), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, go_to_sleep_internal_uninitialized)
{
    EXPECT_THROW(master.GoToSleepInternal(), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, wake_up_uninitialized)
{
    EXPECT_THROW(master.Wakeup(), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, wake_up_internal_uninitialized)
{
    EXPECT_THROW(master.WakeupInternal(), SilKit::StateError);
}


TEST_F(Test_LinControllerDetailedSim, update_tx_buffer_unitialized)
{
    EXPECT_THROW(master.UpdateTxBuffer({}), SilKit::StateError);
}

TEST_F(Test_LinControllerDetailedSim, update_tx_buffer_not_configured_for_tx)
{
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    LinFrame frame = MakeFrame(19, LinChecksumModel::Enhanced);
    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::Rx;
    config.frameResponses = std::vector<LinFrameResponse>{response};
    master.Init(config);

    EXPECT_THROW(master.UpdateTxBuffer(frame), SilKit::ConfigurationError);
}

TEST_F(Test_LinControllerDetailedSim, update_tx_buffer)
{
    auto config = MakeControllerConfig(LinControllerMode::Slave);
    LinFrame frame = MakeFrame(19, LinChecksumModel::Enhanced);
    frame.dataLength = 8;
    frame.data = {1, 2, 3, 4, 5, 6, 7, 8};

    LinFrameResponse response;
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    config.frameResponses.push_back(response);
    EXPECT_CALL(participant, SendMsg(&master, A<const WireLinControllerConfig&>()));
    master.Init(config);

    EXPECT_EQ(master.GetThisLinNode().responses[frame.id].frame.data, frame.data);
    
    frame.data = {1, 1, 1, 1, 1, 1, 1, 1};
    EXPECT_CALL(participant, SendMsg(&master, A<const LinFrameResponseUpdate&>())).Times(1);
    master.UpdateTxBuffer(frame);

    EXPECT_EQ(master.GetThisLinNode().responses[frame.id].frame.data, frame.data);
}

} // namespace
