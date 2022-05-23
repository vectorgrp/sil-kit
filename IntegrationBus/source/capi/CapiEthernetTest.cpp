// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/eth/all.hpp"
#include "EthDatatypeUtils.hpp"
#include "MockParticipant.hpp"

namespace {
using namespace ib::sim::eth;
using ib::mw::test::DummyParticipant;

MATCHER_P(EthFrameMatcher, controlFrame, "") 
{
    *result_listener << "matches ethernet frames by their content and length";
    auto frame1 = controlFrame;
    auto frame2 = arg;
    if (frame1.raw.size() != frame2.raw.size())
    {
        return false;
    }
    for (size_t i = 0; i < frame1.raw.size(); i++)
    {
        if (frame1.raw[i] != frame2.raw[i])
        {
            return false;
        }
    }
    return true;
}

class MockEthernetController : public ib::sim::eth::IEthernetController {
public:
    MOCK_METHOD0(Activate, void());
    MOCK_METHOD0(Deactivate, void());
    MOCK_METHOD1(AddFrameHandler, void(FrameHandler));
    MOCK_METHOD1(AddFrameTransmitHandler, void(FrameTransmitHandler));
    MOCK_METHOD1(AddStateChangeHandler, void(StateChangeHandler));
    MOCK_METHOD1(AddBitrateChangeHandler, void(BitrateChangeHandler));
    MOCK_METHOD1(SendFrame, EthernetTxId(EthernetFrame));
};

class CapiEthernetTest : public testing::Test
{
public: 
    MockEthernetController mockController;
    CapiEthernetTest() 
    {
    }
};

void FrameTransmitHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_FrameTransmitEvent* /*cAck*/)
{
}

void FrameHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_FrameEvent* /*metaData*/)
{
}

void StateChangeHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_StateChangeEvent /*stateChangeEvent*/)
{
}

void BitrateChangeHandler(void* /*context*/, ib_Ethernet_Controller* /*controller*/, ib_Ethernet_BitrateChangeEvent /*bitrateChangeEvent*/)
{
}

    class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(CreateEthernetController, ib::sim::eth::IEthernetController*(const std::string& /*canonicalName*/,
                                                                    const std::string& /*networkName*/));
};

TEST_F(CapiEthernetTest, ethernet_controller_function_mapping)
{
    std::array<uint8_t, 60> buffer;
    ib_Ethernet_Frame ef = { buffer.data(), buffer.size() };

    ib_ReturnCode returnCode;

    MockParticipant mockParticipant;
    EXPECT_CALL(mockParticipant, CreateEthernetController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
    ib_Ethernet_Controller* testParam;
    returnCode = ib_Ethernet_Controller_Create(&testParam, (ib_Participant*)&mockParticipant, "ControllerName",
                                          "NetworkName");
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Activate()).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_Activate((ib_Ethernet_Controller*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Deactivate()).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_Deactivate((ib_Ethernet_Controller*)&mockController);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_AddFrameHandler((ib_Ethernet_Controller*)&mockController, NULL, &FrameHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_AddFrameTransmitHandler((ib_Ethernet_Controller*)&mockController, NULL, &FrameTransmitHandler);
    auto errorString = ib_GetLastErrorString();
    std::cout << "Received Error message: " << returnCode << " " << errorString << std::endl;
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddStateChangeHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_AddStateChangeHandler((ib_Ethernet_Controller*)&mockController, NULL, &StateChangeHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddBitrateChangeHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_AddBitrateChangeHandler((ib_Ethernet_Controller*)&mockController, NULL, &BitrateChangeHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EthernetFrame referenceFrame{};
    referenceFrame.raw = { buffer.data(), buffer.data() + buffer.size() };
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(referenceFrame))).Times(testing::Exactly(1));
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

    returnCode = ib_Ethernet_Controller_AddFrameHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_AddFrameHandler(nullptr, NULL, &FrameHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_AddFrameTransmitHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_AddFrameTransmitHandler(nullptr, NULL, &FrameTransmitHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_AddStateChangeHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_AddStateChangeHandler(nullptr, NULL, &StateChangeHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Ethernet_Controller_AddBitrateChangeHandler((ib_Ethernet_Controller*)&mockController, NULL, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Ethernet_Controller_AddBitrateChangeHandler(nullptr, NULL, &BitrateChangeHandler);
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

    ib_Ethernet_Frame ef = { (const uint8_t*)buffer, PAYLOAD_OFFSET + payloadSize };

    EthernetFrame referenceFrame{};
    referenceFrame.raw = { ef.data, ef.data + ef.size };
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(referenceFrame))).Times(testing::Exactly(1));
    returnCode = ib_Ethernet_Controller_SendFrame((ib_Ethernet_Controller*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

}

}
