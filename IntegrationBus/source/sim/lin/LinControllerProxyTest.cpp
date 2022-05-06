// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
    : proxy(&participant)
    , proxy2(&participant)
    , proxyVibe(&participant)
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

        proxy.SetServiceDescriptor(from_endpointAddress(addr1_proxy));
        proxy2.SetServiceDescriptor(from_endpointAddress(addr2_proxy));
        proxyVibe.SetServiceDescriptor(from_endpointAddress(addr1_vibe));
    }

protected:
    const EndpointAddress addr1_vibe{4, 5};
    const EndpointAddress addr1_proxy{7, 5};
    const EndpointAddress addr2_proxy{4, 9};

    LinMockParticipant participant;
    LinControllerProxy proxy;
    LinControllerProxy proxy2;
    // FIXME check, if this should be a VIBE controller
    LinControllerProxy proxyVibe;
    Callbacks callbacks;
    LinControllerProxy::FrameStatusHandler frameStatusHandler;
    LinControllerProxy::GoToSleepHandler goToSleepHandler;
    LinControllerProxy::WakeupHandler wakeupHandler;
    LinControllerProxy::FrameResponseUpdateHandler frameResponseUpdateHandler;
};

TEST_F(LinControllerProxyTest, send_frame)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,5,6,7,8});
    expectedMsg.responseType = LinFrameResponseType::SlaveResponse;

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedMsg)).Times(1);

    proxy.SendFrame(expectedMsg.frame, expectedMsg.responseType);
}

TEST_F(LinControllerProxyTest, send_frame_header)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);

    LinSendFrameHeaderRequest expectedMsg;
    expectedMsg.id = 13;

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedMsg)).Times(1);

    proxy.SendFrameHeader(expectedMsg.id);
}

TEST_F(LinControllerProxyTest, call_frame_status_handler)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);

    // Receive LinTransmission
    LinFrame rxFrame = MakeFrame(17, LinChecksumModel::Enhanced, 4, {1,2,3,4,0,0,0,0});

    // Expect LIN_RX_OK
    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, rxFrame, LinFrameStatus::LIN_RX_OK)).Times(1);
    LinTransmission transmission;
    transmission.frame = rxFrame;
    transmission.status = LinFrameStatus::LIN_RX_OK;
    proxy.ReceiveIbMessage(&proxyVibe, transmission);
}

TEST_F(LinControllerProxyTest, set_frame_response)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);
    

    LinFrameResponse response;
    response.frame = MakeFrame(19, LinChecksumModel::Enhanced);
    response.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponseUpdate expectedMsg;
    expectedMsg.frameResponses.push_back(response);

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedMsg)).Times(1);

    proxy.SetFrameResponse(response.frame, response.responseMode);
}

TEST_F(LinControllerProxyTest, set_frame_responses)
{
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);


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

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedMsg)).Times(1);

    proxy.SetFrameResponses(responses);
}

TEST_F(LinControllerProxyTest, trigger_frame_response_update_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);

    proxy.AddFrameResponseUpdateHandler(frameResponseUpdateHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    LinFrameResponseUpdate responseUpdate;
    responseUpdate.frameResponses.push_back(response1);
    responseUpdate.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, to_string(proxy2.GetServiceDescriptor()), response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, to_string(proxy2.GetServiceDescriptor()), response2))
        .Times(1);

    proxy.ReceiveIbMessage(&proxy2, responseUpdate);
}

TEST_F(LinControllerProxyTest, trigger_frame_response_update_handler_for_slave_config)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);

    proxy.AddFrameResponseUpdateHandler(frameResponseUpdateHandler);

    LinFrameResponse response1;
    response1.frame = MakeFrame(17, LinChecksumModel::Enhanced);
    response1.responseMode = LinFrameResponseMode::Rx;
    LinFrameResponse response2;
    response2.frame = MakeFrame(19, LinChecksumModel::Classic);
    response2.responseMode = LinFrameResponseMode::TxUnconditional;

    LinControllerConfig slaveCfg = MakeControllerConfig(LinControllerMode::Slave);
    slaveCfg.frameResponses.push_back(response1);
    slaveCfg.frameResponses.push_back(response2);

    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, to_string(proxy2.GetServiceDescriptor()), response1))
        .Times(1);
    EXPECT_CALL(callbacks, FrameResponseUpdateHandler(&proxy, to_string(proxy2.GetServiceDescriptor()), response2))
        .Times(1);

    proxy.ReceiveIbMessage(&proxy2, slaveCfg);
}

TEST_F(LinControllerProxyTest, go_to_sleep)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);


    LinSendFrameRequest expectedMsg;
    expectedMsg.frame = GoToSleepFrame();
    expectedMsg.responseType = LinFrameResponseType::MasterResponse;

    EXPECT_CALL(participant, SendIbMessage(&proxy, expectedMsg))
        .Times(1);
    EXPECT_CALL(participant, SendIbMessage(&proxy, AControllerStatusUpdateWith(LinControllerStatus::SleepPending)))
        .Times(1);

    proxy.GoToSleep();
}

TEST_F(LinControllerProxyTest, go_to_sleep_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);

    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinSendFrameRequest&>()))
        .Times(0);
    EXPECT_CALL(participant, SendIbMessage(&proxy, AControllerStatusUpdateWith(LinControllerStatus::Sleep)))
        .Times(1);

    proxy.GoToSleepInternal();
}

TEST_F(LinControllerProxyTest, call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);
    proxy.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&proxy)).Times(1);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    proxy.ReceiveIbMessage(&proxyVibe, goToSleep);
}

TEST_F(LinControllerProxyTest, not_call_gotosleep_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);
    proxy.AddGoToSleepHandler(goToSleepHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, A<const LinFrame&>(), _)).Times(1);
    EXPECT_CALL(callbacks, GoToSleepHandler(&proxy)).Times(0);

    LinTransmission goToSleep;
    goToSleep.frame = GoToSleepFrame();
    goToSleep.frame.data[0] = 1;
    goToSleep.status = LinFrameStatus::LIN_RX_OK;

    proxy.ReceiveIbMessage(&proxyVibe, goToSleep);
}

TEST_F(LinControllerProxyTest, wake_up)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);

    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinWakeupPulse&>()))
        .Times(1);
    EXPECT_CALL(participant, SendIbMessage(&proxy, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    proxy.Wakeup();
}

TEST_F(LinControllerProxyTest, wake_up_internal)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Master);
    proxy.Init(config);


    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinWakeupPulse&>()))
        .Times(0);
    EXPECT_CALL(participant, SendIbMessage(&proxy, AControllerStatusUpdateWith(LinControllerStatus::Operational)))
        .Times(1);

    proxy.WakeupInternal();
}


TEST_F(LinControllerProxyTest, call_wakeup_handler)
{
    // Configure Master
    EXPECT_CALL(participant, SendIbMessage(&proxy, A<const LinControllerConfig&>()));
    LinControllerConfig config = MakeControllerConfig(LinControllerMode::Slave);
    proxy.Init(config);
    proxy.AddFrameStatusHandler(frameStatusHandler);
    proxy.AddWakeupHandler(wakeupHandler);

    EXPECT_CALL(callbacks, FrameStatusHandler(&proxy, A<const LinFrame&>(), _)).Times(0);
    EXPECT_CALL(callbacks, WakeupHandler(&proxy)).Times(1);

    LinWakeupPulse wakeupPulse;

    proxy.ReceiveIbMessage(&proxyVibe, wakeupPulse);
}

// No initialization causes exception
TEST_F(LinControllerProxyTest, go_to_sleep_uninitialized)
{
    EXPECT_THROW(proxy.GoToSleep(), ib::StateError);
}

TEST_F(LinControllerProxyTest, go_to_sleep_internal_uninitialized)
{
    EXPECT_THROW(proxy.GoToSleepInternal(), ib::StateError);
}

TEST_F(LinControllerProxyTest, wake_up_uninitialized)
{
    EXPECT_THROW(proxy.Wakeup(), ib::StateError);
}

TEST_F(LinControllerProxyTest, wake_up_internal_uninitialized)
{
    EXPECT_THROW(proxy.WakeupInternal(), ib::StateError);
}

} // namespace
