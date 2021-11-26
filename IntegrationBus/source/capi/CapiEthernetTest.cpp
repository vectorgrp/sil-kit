// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/eth/all.hpp"
#include "EthDatatypeUtils.hpp"

namespace {
using namespace ib::sim::eth;

MATCHER_P(EthFrameMatcher, controlFrame, "matches ethernet frames by their content and length") {
    auto frame1 = controlFrame.RawFrame();
    auto frame2 = arg.RawFrame();
    if (frame1.size() != frame2.size()) {
        return false;
    }
    for (int i = 0; i < frame1.size(); i++) {
        if (frame1[i] != frame2[i]) {
            return false;
        }
    }
    return true;
}

class MockEthernetController : public ib::sim::eth::IEthController {
public:
    MOCK_METHOD0(Activate, void());
    MOCK_METHOD0(Deactivate, void());
    MOCK_METHOD1(RegisterReceiveMessageHandler, void(ReceiveMessageHandler));
    MOCK_METHOD1(RegisterMessageAckHandler, void(MessageAckHandler));
    MOCK_METHOD1(RegisterStateChangedHandler, void(StateChangedHandler));
    MOCK_METHOD1(RegisterBitRateChangedHandler, void(BitRateChangedHandler));
    MOCK_METHOD1(SendFrame, EthTxId(EthFrame));
    MOCK_METHOD2(SendFrame, EthTxId(EthFrame, std::chrono::nanoseconds));
    MOCK_METHOD1(SendMessage, EthTxId(EthMessage));
};

class CapiEthernetTest : public testing::Test
{
public: 
    MockEthernetController mockController;
	CapiEthernetTest() 
	{
			
	}
};

void AckCallback(void* context, ib_EthernetController* controller, struct ib_EthernetTransmitAcknowledge* cAck)
{
}

void ReceiveMessage(void* context, ib_EthernetController* controller, ib_EthernetMessage* metaData)
{
}

void StateChangedHandler(void* context, ib_EthernetController* controller, ib_EthernetState state)
{
}

void BitRateChangedHandler(void* context, ib_EthernetController* controller, uint32_t bitrate)
{
}

TEST_F(CapiEthernetTest, ethernet_controller_function_mapping)
{
    ib_EthernetFrame ef = { 0,0 };

    ib_ReturnCode returnCode;

    EXPECT_CALL(mockController, Activate()).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_Activate((ib_EthernetController*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Deactivate()).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_Deactivate((ib_EthernetController*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterReceiveMessageHandler((ib_EthernetController*)&mockController, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterMessageAckHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterFrameAckHandler((ib_EthernetController*)&mockController, NULL, &AckCallback);
    auto errorString = ib_GetLastErrorString();
    std::cout << "Received Error message: " << returnCode << " " << errorString << std::endl;
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterStateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterStateChangedHandler((ib_EthernetController*)&mockController, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterBitRateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterBitRateChangedHandler((ib_EthernetController*)&mockController, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EthFrame refFrame{};
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_SendFrame((ib_EthernetController*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiEthernetTest, ethernet_controller_nullptr_error)
{

    ib_ReturnCode returnCode;

    returnCode = ib_EthernetController_Activate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


    returnCode = ib_EthernetController_Deactivate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterReceiveMessageHandler((ib_EthernetController*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterReceiveMessageHandler(nullptr, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterFrameAckHandler((ib_EthernetController*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterFrameAckHandler(nullptr, NULL, &AckCallback);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterStateChangedHandler((ib_EthernetController*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterStateChangedHandler(nullptr, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterBitRateChangedHandler((ib_EthernetController*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterBitRateChangedHandler(nullptr, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_EthernetFrame ef{0,0};
    returnCode = ib_EthernetController_SendFrame((ib_EthernetController*)&mockController, nullptr, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_SendFrame(nullptr, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}


TEST_F(CapiEthernetTest, ethernet_controller_send_frame)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    const uint8_t PAYLOAD_OFFSET = 14;
    uint8_t buffer[100];

    // set destination mac
    uint8_t destinationMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1 };
    memcpy(&(buffer[0]), destinationMac, sizeof(destinationMac));

    // set source mac
    uint8_t sourceMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2 };
    memcpy(&(buffer[6]), sourceMac, sizeof(sourceMac));

    // set ethertype
    buffer[12] = 0x00;
    buffer[13] = 0x08;

    // set payload
    int ethernetMessageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer + PAYLOAD_OFFSET, sizeof(buffer) - PAYLOAD_OFFSET, "ETHERNET %i", ethernetMessageCounter);

    ib_EthernetFrame ef = { (const uint8_t* const)buffer, PAYLOAD_OFFSET + payloadSize };

    EthFrame refFrame{};
    std::vector<uint8_t> rawFrame(ef.pointer, ef.pointer + ef.size);
    refFrame.SetRawFrame(rawFrame);
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_SendFrame((ib_EthernetController*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

}

}