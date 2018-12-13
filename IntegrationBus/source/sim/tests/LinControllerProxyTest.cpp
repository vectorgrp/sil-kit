// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

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
using namespace std::placeholders;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib::mw;
using namespace ib::sim::lin;

class LinControllerProxyTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveMessageStatus, void(ILinController*, MessageStatus));
        MOCK_METHOD2(ReceiveMessage, void(ILinController*, const LinMessage&));
    };

protected:
    LinControllerProxyTest()
    : proxy(&comAdapter)
    {
        proxy.SetEndpointAddress(proxyAddress);
    }

protected:
    const EndpointAddress controllerAddress {4, 5};
    const EndpointAddress proxyAddress {7, 5};
    const EndpointAddress otherControllerAddress{ 4, 10 };

    ib::mw::test::MockComAdapter comAdapter;
    LinControllerProxy proxy;
    Callbacks callbacks;
};

inline Payload createPayload(const uint8_t a_size, const std::array<uint8_t, 8>& a_data)
{
    Payload payload;
    payload.size = a_size;
    payload.data = a_data;
    return payload;
}

TEST_F(LinControllerProxyTest, send_lin_message)
{
    LinMessage msg;
    msg.status = MessageStatus::TxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, msg)).Times(1);

    proxy.SendMessage(msg);
}

TEST_F(LinControllerProxyTest, request_lin_message)
{
    RxRequest request;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, request))
        .Times(1);

    proxy.RequestMessage(request);
}

/*! \brief Passing a LinMessage to an LinControllerProxys musst trigger the registered callback
 */
TEST_F(LinControllerProxyTest, trigger_callback_on_receive_message)
{
    proxy.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

    LinMessage masterTx;
    masterTx.status = MessageStatus::TxSuccess;

    LinMessage slaveReply;
    slaveReply.status = MessageStatus::RxSuccess;

    EXPECT_CALL(callbacks, ReceiveMessage(&proxy, masterTx)).Times(1);
    EXPECT_CALL(callbacks, ReceiveMessage(&proxy, slaveReply)).Times(1);

    proxy.ReceiveIbMessage(controllerAddress, masterTx);
    proxy.ReceiveIbMessage(controllerAddress, slaveReply);
}

TEST_F(LinControllerProxyTest, ignore_messages_for_other_proxies)
{
    proxy.RegisterReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessage));

    LinMessage masterTx;
    masterTx.status = MessageStatus::TxSuccess;

    LinMessage slaveReply;
    slaveReply.status = MessageStatus::RxSuccess;

    EXPECT_CALL(callbacks, ReceiveMessage(&proxy, A<const LinMessage&>())).Times(0);

    proxy.ReceiveIbMessage(otherControllerAddress, masterTx);
    proxy.ReceiveIbMessage(otherControllerAddress, slaveReply);
}

TEST_F(LinControllerProxyTest, trigger_callback_on_tx_ack)
{
    proxy.SetMasterMode();
    proxy.RegisterTxCompleteHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageStatus));

    TxAcknowledge txAck;
    txAck.status = MessageStatus::TxSuccess;

    EXPECT_CALL(callbacks, ReceiveMessageStatus(&proxy, txAck.status)).Times(1);

    proxy.ReceiveIbMessage(controllerAddress, txAck);
}

TEST_F(LinControllerProxyTest, ignore_acks_for_other_proxies)
{
    proxy.RegisterTxCompleteHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveMessageStatus));

    TxAcknowledge txAck;
    txAck.status = MessageStatus::TxSuccess;

    EXPECT_CALL(callbacks, ReceiveMessageStatus(&proxy, A<MessageStatus>())).Times(0);

    proxy.ReceiveIbMessage(otherControllerAddress, txAck);
}


TEST_F(LinControllerProxyTest, send_baudrate_config)
{
    ControllerConfig config;
    config.controllerMode = ControllerMode::Inactive;
    config.baudrate = 25000;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, config))
        .Times(1);

    proxy.SetBaudRate(config.baudrate);
}

TEST_F(LinControllerProxyTest, set_master_mode)
{
    ControllerConfig config;
    config.controllerMode = ControllerMode::Master;
    config.baudrate = 0;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, config))
        .Times(1);

    LinMessage msg;
    proxy.SetMasterMode();
}

TEST_F(LinControllerProxyTest, set_slave_mode)
{
    ControllerConfig config;
    config.controllerMode = ControllerMode::Slave;
    config.baudrate = 0;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, config))
        .Times(1);

    LinMessage msg;
    proxy.SetSlaveMode();
}

TEST_F(LinControllerProxyTest, propagate_new_response)
{
    proxy.SetSlaveMode();

    Payload payload{4,{1,2,3,4,5,6,7,8}};

    SlaveResponse expectedResponse;
    expectedResponse.linId = 17;
    expectedResponse.payload = payload;
    expectedResponse.checksumModel = ChecksumModel::Undefined;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, expectedResponse))
        .Times(1);

    proxy.SetResponse(17, payload);
}

TEST_F(LinControllerProxyTest, propagate_new_response_with_checksummodel)
{
    proxy.SetSlaveMode();

    Payload payload{4,{1,2,3,4,5,6,7,8}};

    SlaveResponse expectedResponse;
    expectedResponse.linId = 17;
    expectedResponse.payload = payload;
    expectedResponse.checksumModel = ChecksumModel::Enhanced;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, expectedResponse))
        .Times(1);

    proxy.SetResponseWithChecksum(17, payload, ChecksumModel::Enhanced);
}

/*! \brief Ensure that master requests are ignored. They are served by the Network Simulator
 *
 * Configuring a response should only cause the response to be forwarded to the
 * Network Simulator. Although no MasterRequest should arrive at a proxy (they are handled by
 * the Network Simulator), a proxy still should not generate any reply by itself.
 */
TEST_F(LinControllerProxyTest, ignore_master_request)
{
    const LinId linId = 42;
    const Payload payload = createPayload(8, {{1, 2, 3, 4, 5, 6, 7, 8}});

    proxy.SetResponse(linId, payload);

    LinMessage masterRequest;
    masterRequest.linId = linId;
    masterRequest.status = MessageStatus::TxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(proxyAddress, A<const LinMessage&>()))
        .Times(0);

    proxy.ReceiveIbMessage(controllerAddress, masterRequest);
}

TEST_F(LinControllerProxyTest, dont_acknowledge_master_transmit)
{
    const LinId linId = 42;

    LinMessage masterTransmit;
    masterTransmit.linId = linId;
    masterTransmit.status = MessageStatus::TxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(controllerAddress, A<const LinMessage&>()))
        .Times(0);

    proxy.ReceiveIbMessage(controllerAddress, masterTransmit);
}

TEST_F(LinControllerProxyTest, dont_respond_on_slave_response)
{
    LinMessage slaveResponse;
    slaveResponse.linId = 42;
    slaveResponse.status = MessageStatus::RxSuccess;

    EXPECT_CALL(comAdapter, SendIbMessage(A<EndpointAddress>(), A<const LinMessage&>()))
        .Times(0);

    proxy.ReceiveIbMessage(controllerAddress, slaveResponse);
}


} // namespace
