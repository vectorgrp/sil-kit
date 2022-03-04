// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/eth/all.hpp"
#include "EthDatatypeUtils.hpp"
#include "MockComAdapter.hpp"

namespace {
using namespace ib::sim::eth;
using ib::mw::test::DummyComAdapter;

MATCHER_P(EthFrameMatcher, controlFrame, "") 
{
    *result_listener << "matches ethernet frames by their content and length";
    auto frame1 = controlFrame.RawFrame();
    auto frame2 = arg.RawFrame();
    if (frame1.size() != frame2.size()) {
        return false;
    }
    for (size_t i = 0; i < frame1.size(); i++) {
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

void AckCallback(void* /*context*/, ib_Ethernet_Controller* /*controller*/, struct ib_Ethernet_TransmitAcknowledge* /*cAck*/)
{
}

void ReceiveMessage(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_Message* /*metaData*/)
{
}

void StateChangedHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_State /*state*/)
{
}

void BitRateChangedHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, uint32_t /*bitrate*/)
{
}

    class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(CreateEthController, ib::sim::eth::IEthController*(const std::string& /*canonicalName*/,
                                                                    const std::string& /*networkName*/));
};

TEST_F(CapiEthernetTest, ethernet_controller_function_mapping)
{
    std::array<uint8_t, 60> buffer;
    ib_Ethernet_Frame ef = { buffer.data(), buffer.size() };

    ib_ReturnCode returnCode;

    MockComAdapter mockComAdapter;
    EXPECT_CALL(mockComAdapter, CreateEthController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
    ib_Ethernet_Controller* testParam;
    returnCode = ib_Ethernet_Controller_Create(&testParam, (ib_SimulationParticipant*)&mockComAdapter, "ControllerName",
                                          "NetworkName");
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Activate()).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_Activate((ib_Ethernet_Controller*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Deactivate()).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_Deactivate((ib_Ethernet_Controller*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_RegisterReceiveMessageHandler((ib_Ethernet_Controller*)&mockController, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterMessageAckHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_RegisterFrameAckHandler((ib_Ethernet_Controller*)&mockController, NULL, &AckCallback);
    auto errorString = ib_GetLastErrorString();
    std::cout << "Received Error message: " << returnCode << " " << errorString << std::endl;
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterStateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_RegisterStateChangedHandler((ib_Ethernet_Controller*)&mockController, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RegisterBitRateChangedHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_RegisterBitRateChangedHandler((ib_Ethernet_Controller*)&mockController, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EthFrame refFrame{};
    refFrame.SetRawFrame({ buffer.data(), buffer.data() + buffer.size() });
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_SendFrame((ib_Ethernet_Controller*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiEthernetTest, ethernet_controller_nullptr_error)
{

    ib_ReturnCode returnCode;

    returnCode = ib_Ethernet_Controller_Activate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


    returnCode = ib_Ethernet_Controller_Deactivate(nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_RegisterReceiveMessageHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_RegisterReceiveMessageHandler(nullptr, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_RegisterFrameAckHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_RegisterFrameAckHandler(nullptr, NULL, &AckCallback);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_RegisterStateChangedHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_RegisterStateChangedHandler(nullptr, NULL, &StateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_RegisterBitRateChangedHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_RegisterBitRateChangedHandler(nullptr, NULL, &BitRateChangedHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_Ethernet_Frame ef{0,0};
    returnCode = ib_Ethernet_Controller_SendFrame((ib_Ethernet_Controller*)&mockController, nullptr, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_SendFrame(nullptr, &ef, NULL);
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
    size_t payloadSize = snprintf((char*)buffer + PAYLOAD_OFFSET,
        sizeof(buffer) - PAYLOAD_OFFSET,
        "This is the demonstration ethernet frame number %i.",
         ethernetMessageCounter);

    ib_Ethernet_Frame ef = { (const uint8_t* const)buffer, PAYLOAD_OFFSET + payloadSize };

    EthFrame refFrame{};
    std::vector<uint8_t> rawFrame(ef.data, ef.data + ef.size);
    refFrame.SetRawFrame(rawFrame);
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_SendFrame((ib_Ethernet_Controller*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

}

}
