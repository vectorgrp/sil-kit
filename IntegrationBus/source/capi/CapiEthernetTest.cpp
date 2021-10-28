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
    ib_EthernetFrame ef;
    ef.frameData = 0;
    ef.frameSize = 0;

    ib_ReturnCode returnCode;

    EXPECT_CALL(mockController, Activate()).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_Activate(&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Deactivate()).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_Deactivate(&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterReceiveMessageHandler(&mockController, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterMessageAckHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterFrameAckHandler(&mockController, NULL, &AckCallback);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterStateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterStateChangedHandler(&mockController, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterBitRateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_RegisterBitRateChangedHandler(&mockController, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EthFrame refFrame{};
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_SendFrame(&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiEthernetTest, ethernet_controller_nullptr_error)
{

    ib_ReturnCode returnCode;

    returnCode = ib_EthernetController_Activate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


    returnCode = ib_EthernetController_Deactivate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterReceiveMessageHandler(&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterReceiveMessageHandler(nullptr, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterFrameAckHandler(&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterFrameAckHandler(nullptr, NULL, &AckCallback);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterStateChangedHandler(&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterStateChangedHandler(nullptr, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_EthernetController_RegisterBitRateChangedHandler(&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_RegisterBitRateChangedHandler(nullptr, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_EthernetFrame ef{0,0};
    returnCode = ib_EthernetController_SendFrame(&mockController, nullptr, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_EthernetController_SendFrame(nullptr, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}


TEST_F(CapiEthernetTest, ethernet_controller_send_frame)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    char buffer[64];
    ib_ByteVector payload;
    int ethernetMessageCounter = 1;
    payload.size = snprintf(buffer, sizeof(buffer), "ETHERNET %i", ethernetMessageCounter);
    payload.pointer = (uint8_t*) buffer;

    // create empty frame and allocate memory
    ib_EthernetFrame ef;
    ef.frameSize = (size_t)payload.size + sizeof(ib_EthernetFrame_Header);
    ef.frameData = (uint8_t*)malloc(ef.frameSize);

    // set destination mac
    uint8_t destinationMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1 };
    memcpy(ef.frameHeader->destinationMac, destinationMac, sizeof destinationMac);

    // set source mac
    uint8_t sourceMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2 };
    memcpy(ef.frameHeader->sourceMac, sourceMac, sizeof sourceMac);

    ef.frameHeader->etherType = 0x0800;

    // copy payload into frame
    memcpy(ef.frameData + sizeof(ib_EthernetFrame_Header), payload.pointer, payload.size);

    EthFrame refFrame{};
    std::vector<uint8_t> rawFrame(ef.frameData, ef.frameData + ef.frameSize);
    refFrame.SetRawFrame(rawFrame);
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_EthernetController_SendFrame(&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    free(ef.frameData);
}

}