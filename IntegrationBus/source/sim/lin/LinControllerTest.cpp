// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/string_utils.hpp"
#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"

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

auto MakeControllerConfig(ControllerMode mode) -> ControllerConfig
{
    ControllerConfig ctrlConfig;
    ctrlConfig.controllerMode = mode;

    return ctrlConfig;
}

auto MakeSlaveConfiguration(std::initializer_list<SlaveResponseConfig> configs) -> SlaveConfiguration
{
    SlaveConfiguration slaveConfig;
    slaveConfig.responseConfigs = configs;
    return slaveConfig;
}

auto MakeSlaveResponseConfig(LinMessage msg, ResponseMode mode) -> SlaveResponseConfig
{
    SlaveResponseConfig responseConfig;
    responseConfig.linId = msg.linId;
    responseConfig.responseMode = mode;
    responseConfig.payloadLength = msg.payload.size;
    responseConfig.checksumModel = msg.checksumModel;
    return responseConfig;
}

auto MakeSlaveResponse(LinMessage msg) -> SlaveResponse
{
    SlaveResponse response;
    response.linId = msg.linId;
    response.payload = msg.payload;
    response.checksumModel = msg.checksumModel;
    return response;
}


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
        comAdapter.logger = spdlog::default_logger();
        controller.SetEndpointAddress(controllerAddress);
        controller.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));
        controller.RegisterTxCompleteHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageStatus));
        controller.RegisterSleepCommandHandler(ib::util::bind_method(&callbacks, &Callbacks::SleepCommand));
        controller.RegisterWakeupRequestHandler(ib::util::bind_method(&callbacks, &Callbacks::WakeupRequest));
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
    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(reply));

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

    EndpointAddress slave1{2,4};
    controller.ReceiveIbMessage(slave1, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(slave1, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(slave1, MakeSlaveResponse(reply));

    EndpointAddress slave2{3,5};
    controller.ReceiveIbMessage(slave2, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(slave2, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(slave2, MakeSlaveResponse(reply));

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

    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(reply));

    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Sleep));

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

    LinMessage noReply{};
    noReply.linId = request.linId;
    noReply.status = MessageStatus::RxNoResponse;
    noReply.checksumModel = ChecksumModel::Undefined;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, reply))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, reply))
        .Times(1);

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, noReply))
        .Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, noReply))
        .Times(1);

    controller.SetMasterMode();
    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(reply));
    controller.RequestMessage(request);

    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(noReply, ResponseMode::Unused)}));
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
    SlaveConfiguration config;
    SlaveResponseConfig responseConfig;
    responseConfig.responseMode = ResponseMode::Rx;
    responseConfig.linId = 0;
    config.responseConfigs.push_back(responseConfig);
    responseConfig.linId = 1;
    config.responseConfigs.push_back(responseConfig);
    controller.SetSlaveConfiguration(config);

    LinMessage masterTx;
    masterTx.status = MessageStatus::TxSuccess;
    masterTx.linId = 0;
    LinMessage slaveTx;
    slaveTx.status = MessageStatus::RxSuccess;
    slaveTx.linId = 1;
    
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

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, goodReply))
        .Times(2);
    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, falseReply))
        .Times(1);

    EXPECT_CALL(callbacks, ReceiveMessage(&controller, goodReply))
        .Times(2);
    EXPECT_CALL(callbacks, ReceiveMessage(&controller, falseReply))
        .Times(1);

    controller.SetMasterMode();
    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(goodReply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(goodReply));
    controller.RequestMessage(request);

    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(falseReply));
    controller.RequestMessage(request);

    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(goodReply));
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

    SlaveConfiguration config;
    SlaveResponseConfig responseConfig;
    responseConfig.responseMode = ResponseMode::Rx;
    responseConfig.linId = 0x3C;
    config.responseConfigs.push_back(responseConfig);
    controller.SetSlaveConfiguration(config);

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


TEST_F(LinControllerTest, configure_master_after_slave)
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

    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(reply));
    controller.SetMasterMode();
    controller.RequestMessage(request);
}

TEST_F(LinControllerTest, set_slave_response_before_slaveconfiguration)
{
    // Ensure that the slave configuration is properly processed even
    // if the middleware does not ensure in-order data delivery.
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
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveResponse(reply));
    controller.ReceiveIbMessage(otherControllerAddress, MakeSlaveConfiguration({MakeSlaveResponseConfig(reply, ResponseMode::TxUnconditional)}));
    controller.ReceiveIbMessage(otherControllerAddress, MakeControllerConfig(ControllerMode::Slave));
    controller.RequestMessage(request);
}


} // anonymous namespace
