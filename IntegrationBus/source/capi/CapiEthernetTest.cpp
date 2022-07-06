// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/eth/all.hpp"
#include "EthDatatypeUtils.hpp"
#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::Ethernet;
using SilKit::Core::Tests::DummyParticipant;

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

class MockEthernetController : public SilKit::Services::Ethernet::IEthernetController {
public:
    MOCK_METHOD0(Activate, void());
    MOCK_METHOD0(Deactivate, void());
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHandler, (FrameHandler));
    MOCK_METHOD(void, RemoveFrameHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler));
    MOCK_METHOD(void, RemoveFrameTransmitHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddStateChangeHandler, (StateChangeHandler));
    MOCK_METHOD(void, RemoveStateChangeHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddBitrateChangeHandler, (BitrateChangeHandler));
    MOCK_METHOD(void, RemoveBitrateChangeHandler, (SilKit::Services::HandlerId));
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

void FrameTransmitHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetFrameTransmitEvent* /*cAck*/)
{
}

void FrameHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetFrameEvent* /*metaData*/)
{
}

void StateChangeHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetStateChangeEvent* /*stateChangeEvent*/)
{
}

void BitrateChangeHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetBitrateChangeEvent* /*bitrateChangeEvent*/)
{
}

    class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(CreateEthernetController, SilKit::Services::Ethernet::IEthernetController*(const std::string& /*canonicalName*/,
                                                                    const std::string& /*networkName*/));
};

TEST_F(CapiEthernetTest, ethernet_controller_function_mapping)
{
    using SilKit::Util::HandlerId;

    std::array<uint8_t, 60> buffer;
    SilKit_EthernetFrame ef = {SilKit_InterfaceIdentifier_EthernetFrame, {buffer.data(), buffer.size()}};

    SilKit_ReturnCode returnCode;
    MockParticipant mockParticipant;
    SilKit_HandlerId handlerId;

    EXPECT_CALL(mockParticipant, CreateEthernetController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
    SilKit_EthernetController* testParam;
    returnCode = SilKit_EthernetController_Create(&testParam, (SilKit_Participant*)&mockParticipant, "ControllerName",
                                          "NetworkName");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Activate()).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_Activate((SilKit_EthernetController*)&mockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Deactivate()).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_Deactivate((SilKit_EthernetController*)&mockController);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL, &FrameHandler,
                                                        &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_RemoveFrameHandler((SilKit_EthernetController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                &FrameTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameTransmitHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_RemoveFrameTransmitHandler((SilKit_EthernetController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddStateChangeHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddStateChangeHandler((SilKit_EthernetController*)&mockController, NULL,
                                                              &StateChangeHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveStateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_RemoveStateChangeHandler((SilKit_EthernetController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddBitrateChangeHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddBitrateChangeHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                &BitrateChangeHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveBitrateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_RemoveBitrateChangeHandler((SilKit_EthernetController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EthernetFrame referenceFrame{};
    referenceFrame.raw = { buffer.data(), buffer.data() + buffer.size() };
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(referenceFrame))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiEthernetTest, ethernet_controller_nullptr_params)
{
    SilKit_ReturnCode returnCode;
    SilKit_HandlerId handlerId;

    returnCode = SilKit_EthernetController_Activate(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_Deactivate(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameHandler(nullptr, NULL, &FrameHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL, &FrameHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL, nullptr,
                                                                &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(nullptr, NULL, &FrameTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                &FrameTransmitHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_AddStateChangeHandler((SilKit_EthernetController*)&mockController, NULL, nullptr,
                                                              &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddStateChangeHandler(nullptr, NULL, &StateChangeHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddStateChangeHandler((SilKit_EthernetController*)&mockController, NULL,
                                                              &StateChangeHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_AddBitrateChangeHandler((SilKit_EthernetController*)&mockController, NULL, nullptr,
                                                                &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddBitrateChangeHandler(nullptr, NULL, &BitrateChangeHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddBitrateChangeHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                &BitrateChangeHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_RemoveFrameHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_RemoveFrameTransmitHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_RemoveStateChangeHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_RemoveBitrateChangeHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_EthernetFrame ef{SilKit_InterfaceIdentifier_EthernetFrame, {0, 0}};
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, nullptr, NULL);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_SendFrame(nullptr, &ef, NULL);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(CapiEthernetTest, ethernet_controller_send_frame)
{
    SilKit_ReturnCode returnCode = 0;
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

    SilKit_EthernetFrame ef = {SilKit_InterfaceIdentifier_EthernetFrame,
                            {(const uint8_t*)buffer, PAYLOAD_OFFSET + payloadSize}};

    EthernetFrame refFrame{};
    std::vector<uint8_t> rawFrame(ef.raw.data, ef.raw.data + ef.raw.size);
    refFrame.raw = rawFrame;
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

}

}
