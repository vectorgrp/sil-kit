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
        MOCK_METHOD1(SleepCommand, void(ILinController*));
        MOCK_METHOD1(WakeupRequest, void(ILinController*));
    };

protected:
    LinControllerTest()
        : controller(&comAdapter)
    {
        controller.SetEndpointAddress(controllerAddress);
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.RegisterTxCompleteHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageStatus));
        controller.RegisterSleepCommandHandler(ib::util::bind_method(&callbacks, &Callbacks::SleepCommand));
        controller.RegisterWakeupRequestHandler(ib::util::bind_method(&callbacks, &Callbacks::WakeupRequest));
    }

    void SetupResponse(EndpointAddress from, LinMessage msg)
    {
        ControllerConfig ctrlConfig;
        ctrlConfig.controllerMode = ControllerMode::Slave;

        SlaveConfiguration slaveConfig;
        SlaveResponseConfig responseConfig;
        responseConfig.linId = msg.linId;
        responseConfig.responseMode = ResponseMode::TxUnconditional;
        responseConfig.payloadLength = msg.payload.size;
        responseConfig.checksumModel = msg.checksumModel;
        slaveConfig.responseConfigs.emplace_back(std::move(responseConfig));

        SlaveResponse response;
        response.linId = msg.linId;
        response.payload = msg.payload;

        controller.ReceiveIbMessage(from, ctrlConfig);
        controller.ReceiveIbMessage(from, slaveConfig);
        controller.ReceiveIbMessage(from, response);
    }

    void RemoveResponse(EndpointAddress from, LinId linId)
    {
        SlaveConfiguration slaveConfig;
        SlaveResponseConfig responseConfig;
        responseConfig.linId = linId;
        responseConfig.responseMode = ResponseMode::Unused;
        responseConfig.checksumModel = ChecksumModel::Undefined;
        responseConfig.payloadLength = 0;
        slaveConfig.responseConfigs.emplace_back(std::move(responseConfig));

        controller.ReceiveIbMessage(from, slaveConfig);
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
    controller.SetMasterMode();

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
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxNoResponse;
    reply.checksumModel = ChecksumModel::Undefined;


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
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxSuccess;
    reply.payload.size = 4;
    reply.payload.data = {{1,2,3,4,5,6,7,8}};
    reply.checksumModel = ChecksumModel::Enhanced;

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
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage reply;
    reply.linId = request.linId;
    reply.status = MessageStatus::RxResponseError;
    reply.payload.size = 4;
    reply.payload.data = {{1,2,3,4,5,6,7,8}};
    reply.checksumModel = ChecksumModel::Enhanced;

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

TEST_F(LinControllerTest, request_lin_message_with_sleeping_slave)
{
    RxRequest request{};
    request.linId = 20;
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxNoResponse;
    reply.checksumModel = ChecksumModel::Undefined;


    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const RxRequest&>()))
        .Times(0);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);

    controller.SetMasterMode();
    SetupResponse(otherControllerAddress, reply);

    ControllerConfig sleepConfig;
    sleepConfig.controllerMode = ControllerMode::Sleep;
    controller.ReceiveIbMessage(otherControllerAddress, sleepConfig);

    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, request_lin_message_after_remove_response)
{
    RxRequest request{};
    request.linId = 20;
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage reply{};
    reply.linId = request.linId;
    reply.status = MessageStatus::RxSuccess;
    reply.payload.size = 4;
    reply.payload.data = {{1,2,3,4,5,6,7,8}};
    reply.checksumModel = ChecksumModel::Enhanced;

    SetupResponse(otherControllerAddress, reply);

    LinMessage noReply{};
    noReply.linId = request.linId;
    noReply.status = MessageStatus::RxNoResponse;
    noReply.checksumModel = ChecksumModel::Undefined;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, noReply))
        .Times(1);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const RxRequest&>()))
        .Times(0);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, noReply))
        .Times(1);


    controller.SetMasterMode();

    SetupResponse(otherControllerAddress, reply);
    controller.RequestMessage(request);

    RemoveResponse(otherControllerAddress, request.linId);
    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, remove_response)
{
    SlaveConfiguration slaveConfig;
    SlaveResponseConfig responseConfig;
    responseConfig.linId = 9;
    responseConfig.responseMode = ResponseMode::Unused;
    responseConfig.checksumModel = ChecksumModel::Undefined;
    responseConfig.payloadLength = 0;
    slaveConfig.responseConfigs.emplace_back(std::move(responseConfig));

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, slaveConfig))
        .Times(1);

    controller.SetSlaveMode();
    controller.RemoveResponse(9);
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

TEST_F(LinControllerTest, propagate_new_response)
{
    controller.SetSlaveMode();

    Payload payload{4, {1,2,3,4,5,6,7,8}};

    SlaveResponse expectedResponse;
    expectedResponse.linId = 17;
    expectedResponse.payload = payload;
    expectedResponse.checksumModel = ChecksumModel::Undefined;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, expectedResponse))
        .Times(1);

    controller.SetResponse(17, payload);
}

TEST_F(LinControllerTest, propagate_new_response_with_checksummodel)
{
    controller.SetSlaveMode();

    Payload payload{4,{1,2,3,4,5,6,7,8}};

    SlaveResponse expectedResponse;
    expectedResponse.linId = 17;
    expectedResponse.payload = payload;
    expectedResponse.checksumModel = ChecksumModel::Enhanced;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, expectedResponse))
        .Times(1);

    controller.SetResponseWithChecksum(17, payload, ChecksumModel::Enhanced);
}

TEST_F(LinControllerTest, enable_false_checksum_emulation)
{
    RxRequest request{};
    request.linId = 20;
    request.checksumModel = ChecksumModel::Enhanced;

    LinMessage goodReply{};
    goodReply.linId = request.linId;
    goodReply.status = MessageStatus::RxSuccess;
    goodReply.payload.size = 4;
    goodReply.payload.data = {{1,2,3,4,5,6,7,8}};
    goodReply.checksumModel = ChecksumModel::Enhanced;

    LinMessage falseReply = goodReply;
    falseReply.checksumModel = ChecksumModel::Classic;

    SlaveResponse configureGoodReply;
    configureGoodReply.linId = goodReply.linId;
    configureGoodReply.payload = goodReply.payload;
    configureGoodReply.checksumModel = goodReply.checksumModel;


    SlaveResponse configureFalseReply;
    configureFalseReply.linId = falseReply.linId;
    configureFalseReply.payload = falseReply.payload;
    configureFalseReply.checksumModel = falseReply.checksumModel;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, goodReply))
        .Times(2);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, falseReply))
        .Times(1);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, goodReply))
        .Times(2);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, falseReply))
        .Times(1);

    controller.SetMasterMode();
    SetupResponse(otherControllerAddress, goodReply);
    controller.RequestMessage(request);

    controller.ReceiveIbMessage(otherControllerAddress, configureFalseReply);
    controller.RequestMessage(request);

    controller.ReceiveIbMessage(otherControllerAddress, configureGoodReply);
    controller.RequestMessage(request);
}



TEST_F(LinControllerTest, send_gotosleep_command)
{
    controller.SetMasterMode();

    LinMessage gotosleepCmd;
    gotosleepCmd.status  = MessageStatus::TxSuccess;
    gotosleepCmd.linId   = LinId{0x3C};
    gotosleepCmd.payload = Payload{8,{0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    gotosleepCmd.checksumModel = ChecksumModel::Classic;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, gotosleepCmd))
        .Times(1);

    controller.SendGoToSleep();
}

TEST_F(LinControllerTest, call_gotosleep_callback)
{
    controller.SetSlaveMode();

    LinMessage gotosleepCmd;
    gotosleepCmd.status = MessageStatus::TxSuccess;
    gotosleepCmd.linId = LinId{0x3C};
    gotosleepCmd.payload = Payload{8, {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    gotosleepCmd.checksumModel = ChecksumModel::Classic;

    EXPECT_CALL(callbacks, SleepCommand(&controller))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, gotosleepCmd))
        .Times(1);

    controller.ReceiveIbMessage(otherControllerAddress, gotosleepCmd);
}

TEST_F(LinControllerTest, set_sleep_mode)
{
    controller.SetSlaveMode();

    ControllerConfig sleepCfg{};
    sleepCfg.controllerMode = ControllerMode::Sleep;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, sleepCfg))
        .Times(1);

    controller.SetSleepMode();
}

TEST_F(LinControllerTest, send_wakeup_request)
{
    controller.SetSlaveMode();

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const WakeupRequest&>()))
        .Times(1);

    controller.SetSleepMode();
    controller.SendWakeupRequest();
}

TEST_F(LinControllerTest, call_wakeup_callback)
{
    controller.SetMasterMode();

    WakeupRequest wakeup;

    EXPECT_CALL(callbacks, WakeupRequest(&controller))
        .Times(1);

    controller.ReceiveIbMessage(otherControllerAddress, wakeup);
}

TEST_F(LinControllerTest, set_master_operational)
{
    controller.SetMasterMode();
    controller.SetSleepMode();

    ControllerConfig operationalCfg{};
    operationalCfg.controllerMode = ControllerMode::Master;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, operationalCfg))
        .Times(1);

    controller.SetOperationalMode();
}

TEST_F(LinControllerTest, set_slave_operational)
{
    controller.SetSlaveMode();
    controller.SetSleepMode();

    ControllerConfig operationalCfg{};
    operationalCfg.controllerMode = ControllerMode::Slave;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, operationalCfg))
        .Times(1);

    controller.SetOperationalMode();
}


} // anonymous namespace
