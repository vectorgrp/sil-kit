// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "MwTestUtils.hpp"

#include "LinDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using ::testing::_;
using ::testing::A;
using ::testing::An;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::lin;

class LinControllerTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessageStatus, void(ILinController*, MessageStatus));
        MOCK_METHOD2(ReceiveMessage, void(ILinController*, const LinMessage&));
    };

protected:
    LinControllerTest()
        : controller(&comAdapter)
    {
        controller.SetEndpointAddress(controllerAddress);
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.RegisterTxCompleteHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageStatus));
    }

    void SetupResponse(EndpointAddress from, LinMessage msg)
    {
        ControllerConfig ctrlConfig;
        ctrlConfig.controllerMode = ControllerMode::Slave;

        SlaveConfiguration config;
        config.responseConfigs.resize(msg.linId + 1);

        auto&& responseConfig = config.responseConfigs[msg.linId];
        responseConfig.responseMode = ResponseMode::TxUnconditional;
        responseConfig.payloadLength = msg.payload.size;
        responseConfig.checksumModel = msg.checksumModel;

        SlaveResponse response;
        response.linId = msg.linId;
        response.payload = msg.payload;

        controller.ReceiveIbMessage(from, ctrlConfig);
        controller.ReceiveIbMessage(from, config);
        controller.ReceiveIbMessage(from, response);
    }

protected:
    const EndpointAddress controllerAddress{4, 5};
    const EndpointAddress otherControllerAddress{5, 10};

    ib::mw::test::MockComAdapter comAdapter;
    LinController controller;
    Callbacks callbacks;
};


TEST_F(LinControllerTest, send_lin_message)
{
    LinMessage msg;
    msg.status = MessageStatus::TxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, msg))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveMessageStatus(&controller, MessageStatus::TxSuccess))
        .Times(1);

    controller.SendMessage(msg);
}

TEST_F(LinControllerTest, request_lin_message_without_configured_response)
{
    RxRequest request{};
    request.linId = 19;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxNoResponse;


    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const RxRequest&>()))
        .Times(0);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);

    controller.SetMasterMode();
    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, request_lin_message_with_one_configured_response)
{
    RxRequest request{};
    request.linId = 20;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxSuccess;
    reply.payload.size = 4;
    reply.payload.data = {{1,2,3,4,5,6,7,8}};

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const RxRequest&>()))
        .Times(0);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);

    controller.SetMasterMode();
    SetupResponse(otherControllerAddress, reply);
    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, request_lin_message_with_multiple_configured_responses)
{
    RxRequest request;
    request.linId = 21;

    LinMessage reply;
    reply.linId = request.linId;
    reply.status = MessageStatus::RxResponseError;
    reply.payload.size = 4;
    reply.payload.data = {{1,2,3,4,5,6,7,8}};

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const RxRequest&>()))
        .Times(0);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);

    controller.SetMasterMode();
    SetupResponse(EndpointAddress{2,4}, reply);
    SetupResponse(EndpointAddress{3,5}, reply);
    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, dont_acknowledge_master_transmit)
{
    controller.SetSlaveMode();

    LinMessage masterTx;
    masterTx.linId = 17;
    masterTx.status = MessageStatus::TxSuccess;

    TxAcknowledge expectedAck;
    expectedAck.linId = masterTx.linId;
    expectedAck.status = MessageStatus::TxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const TxAcknowledge&>()))
        .Times(0);

    controller.ReceiveIbMessage(otherControllerAddress, masterTx);
}

TEST_F(LinControllerTest, dont_acknowledge_slave_responses)
{
    controller.SetSlaveMode();

    LinMessage slaveTx;
    slaveTx.linId = 17;
    slaveTx.status = MessageStatus::RxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const TxAcknowledge&>()))
        .Times(0);

    controller.ReceiveIbMessage(otherControllerAddress, slaveTx);
}

TEST_F(LinControllerTest, trigger_slave_callbacks)
{
    controller.SetSlaveMode();


    LinMessage masterTx;
    masterTx.status = MessageStatus::TxSuccess;
    
    LinMessage slaveTx;
    slaveTx.status = MessageStatus::RxSuccess;
    
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, masterTx)).Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, slaveTx)).Times(1);

    controller.ReceiveIbMessage(otherControllerAddress, masterTx);
    controller.ReceiveIbMessage(otherControllerAddress, slaveTx);
}



} // anonymous namespace
