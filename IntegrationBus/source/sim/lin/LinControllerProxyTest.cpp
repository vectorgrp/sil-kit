// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

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
using namespace std::placeholders;

using namespace testing;
using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::lin;
using namespace ib::sim::lin::test;
using namespace ib::util;

class LinControllerProxyTest : public testing::Test
{

protected:
    LinControllerProxyTest()
    : proxy(&comAdapter)
    {
        frameStatusHandler =
            [this](ILinController* ctrl, const Frame& frame, FrameStatus status, std::chrono::nanoseconds) {
            callbacks.FrameStatusHandler(ctrl, frame, status);
        };

        proxy.SetEndpointAddress(addr1_proxy);
    }

protected:
    const EndpointAddress addr1_vibe{4, 5};
    const EndpointAddress addr1_proxy{7, 5};
    const EndpointAddress addr2_proxy{4, 9};

    LinMockComAdapter comAdapter;
    LinControllerProxy proxy;
    Callbacks callbacks;
    LinControllerProxy::FrameStatusHandler frameStatusHandler;
};

TEST_F(LinControllerProxyTest, send_frame)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);
    proxy.RegisterFrameStatusHandler(frameStatusHandler);

    SendFrameRequest expectedMsg;
    expectedMsg.frame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    expectedMsg.responseType = FrameResponseType::SlaveResponse;

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, expectedMsg)).Times(1);

    proxy.SendFrame(expectedMsg.frame, expectedMsg.responseType);
}

TEST_F(LinControllerProxyTest, send_frame_header)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);
    proxy.RegisterFrameStatusHandler(frameStatusHandler);

    SendFrameHeaderRequest expectedMsg;
    expectedMsg.id = 13;

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, expectedMsg)).Times(1);

    proxy.SendFrameHeader(expectedMsg.id);
}

TEST_F(LinControllerProxyTest, call_frame_status_handler)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    proxy.Init(config);
    proxy.RegisterFrameStatusHandler(frameStatusHandler);

    // Receive Transmission
    Frame rxFrame = MakeFrame(17, ChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});

    // Expect LIN_RX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, rxFrame, FrameStatus::LIN_RX_OK)).Times(1);
    Transmission transmission;
    transmission.frame = rxFrame;
    transmission.status = FrameStatus::LIN_RX_OK;
    proxy.ReceiveIbMessage(addr1_vibe, transmission);
}

TEST_F(LinControllerProxyTest, set_frame_response)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    proxy.Init(config);
    

    FrameResponse response;
    response.frame = MakeFrame(19, ChecksumModel::Enhanced);
    response.responseMode = FrameResponseMode::Rx;

    FrameResponseUpdate expectedMsg;
    expectedMsg.frameResponses.push_back(response);

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, expectedMsg)).Times(1);

    proxy.SetFrameResponse(response.frame, response.responseMode);
}

TEST_F(LinControllerProxyTest, set_frame_responses)
{
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    proxy.Init(config);


    FrameResponse response1;
    response1.frame = MakeFrame(19, ChecksumModel::Enhanced);
    response1.responseMode = FrameResponseMode::Rx;

    FrameResponse response2;
    response2.frame = MakeFrame(3, ChecksumModel::Classic, 4, std::array<uint8_t, 8>{1, 2, 3, 4, 1, 2, 3, 4});
    response2.responseMode = FrameResponseMode::TxUnconditional;

    std::vector<FrameResponse> responses;
    responses.push_back(response1);
    responses.push_back(response2);


    FrameResponseUpdate expectedMsg;
    expectedMsg.frameResponses = responses;

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, expectedMsg)).Times(1);

    proxy.SetFrameResponses(responses);
}


TEST_F(LinControllerProxyTest, trigger_frame_response_update_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);

    proxy.RegisterFrameResponseUpdateHandler(bind_method(&callbacks, &Callbacks::FrameResponseUpdateHandler));

    FrameResponse response1;
    response1.frame = MakeFrame(17, ChecksumModel::Enhanced);
    response1.responseMode = FrameResponseMode::Rx;

    FrameResponse response2;
    response2.frame = MakeFrame(19, ChecksumModel::Classic);
    response2.responseMode = FrameResponseMode::TxUnconditional;

    FrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.push_back(response1);
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, addr2_proxy, response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, addr2_proxy, response2))
        .Times(1);

    proxy.ReceiveIbMessage(addr2_proxy, responseUpdate);
}

TEST_F(LinControllerProxyTest, trigger_frame_response_update_handler_for_slave_config)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);

    proxy.RegisterFrameResponseUpdateHandler(bind_method(&callbacks, &Callbacks::FrameResponseUpdateHandler));

    FrameResponse response1;
    response1.frame = MakeFrame(17, ChecksumModel::Enhanced);
    response1.responseMode = FrameResponseMode::Rx;
    FrameResponse response2;
    response2.frame = MakeFrame(19, ChecksumModel::Classic);
    response2.responseMode = FrameResponseMode::TxUnconditional;

    ControllerConfig slaveCfg = MakeControllerConfig(ControllerMode::Slave);
    slaveCfg.frameResponses.push_back(response1);
    slaveCfg.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, addr2_proxy, response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, addr2_proxy, response2))
        .Times(1);

    proxy.ReceiveIbMessage(addr2_proxy, slaveCfg);
}

TEST_F(LinControllerProxyTest, got_to_sleep)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);


    SendFrameRequest expectedMsg;
    expectedMsg.frame = GoToSleepFrame();
    expectedMsg.responseType = FrameResponseType::MasterResponse;

    ControllerStatusUpdate expectedStatus;
    expectedStatus.status = ControllerStatus::Sleep;

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, expectedMsg))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, AControllerStatusUpdateWith(ControllerStatus::Sleep)))
        .Times(1);

    proxy.GoToSleep();
}

TEST_F(LinControllerProxyTest, got_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const SendFrameRequest&>()))
        .Times(0);
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, AControllerStatusUpdateWith(ControllerStatus::Sleep)))
        .Times(1);

    proxy.GoToSleepInternal();
}

TEST_F(LinControllerProxyTest, call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    proxy.Init(config);
    proxy.RegisterFrameStatusHandler(frameStatusHandler);
    proxy.RegisterGoToSleepHandler(bind_method(&callbacks, &Callbacks::GoToSleepHandler));

    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, A<const Frame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&proxy)).Times(1);

    Transmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = FrameStatus::LIN_RX_OK;

    proxy.ReceiveIbMessage(addr1_vibe, goToSleep);
}

TEST_F(LinControllerProxyTest, wake_up)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);

    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const WakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, AControllerStatusUpdateWith(ControllerStatus::Operational)))
        .Times(1);

    proxy.Wakeup();
}

TEST_F(LinControllerProxyTest, wake_up_internal)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Master);
    proxy.Init(config);


    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const WakeupPulse&>()))
        .Times(0);
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, AControllerStatusUpdateWith(ControllerStatus::Operational)))
        .Times(1);

    proxy.WakeupInternal();
}


TEST_F(LinControllerProxyTest, call_wakeup_handler)
{
    // Configure Master
    EXPECT_CALL(comAdapter, SendIbMessage(addr1_proxy, A<const ControllerConfig&>()));
    ControllerConfig config = MakeControllerConfig(ControllerMode::Slave);
    proxy.Init(config);
    proxy.RegisterFrameStatusHandler(frameStatusHandler);
    proxy.RegisterWakeupHandler(bind_method(&callbacks, &Callbacks::WakeupHandler));

    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, A<const Frame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&proxy)).Times(1);

    WakeupPulse wakeupPulse;

    proxy.ReceiveIbMessage(addr1_vibe, wakeupPulse);
}


} // namespace
