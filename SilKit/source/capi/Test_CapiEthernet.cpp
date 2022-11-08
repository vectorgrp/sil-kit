/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/ethernet/all.hpp"
#include "EthDatatypeUtils.hpp"
#include "MockParticipant.hpp"

#include "fmt/format.h"

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
    MOCK_METHOD(void, Activate, (), (override));
    MOCK_METHOD(void, Deactivate, (), (override));
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHandler, (FrameHandler, SilKit::Services::DirectionMask), (override));
    MOCK_METHOD(void, RemoveFrameHandler, (SilKit::Services::HandlerId), (override));
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler, EthernetTransmitStatusMask), (override));
    MOCK_METHOD(void, RemoveFrameTransmitHandler, (SilKit::Services::HandlerId), (override));
    MOCK_METHOD(SilKit::Services::HandlerId, AddStateChangeHandler, (StateChangeHandler), (override));
    MOCK_METHOD(void, RemoveStateChangeHandler, (SilKit::Services::HandlerId), (override));
    MOCK_METHOD(SilKit::Services::HandlerId, AddBitrateChangeHandler, (BitrateChangeHandler), (override));
    MOCK_METHOD(void, RemoveBitrateChangeHandler, (SilKit::Services::HandlerId), (override));
    MOCK_METHOD(void, SendFrame, (EthernetFrame, void*), (override));
};

class CapiEthernetTest : public testing::Test
{
public: 
    MockEthernetController mockController;
    CapiEthernetTest() 
    {
    }
};

void SilKitCALL FrameTransmitHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetFrameTransmitEvent* /*cAck*/)
{
}

void SilKitCALL FrameHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetFrameEvent* /*metaData*/)
{
}

void SilKitCALL StateChangeHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetStateChangeEvent* /*stateChangeEvent*/)
{
}

void SilKitCALL BitrateChangeHandler(void* /*context*/, SilKit_EthernetController* /*controller*/, SilKit_EthernetBitrateChangeEvent* /*bitrateChangeEvent*/)
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
    SilKit_EthernetFrame ef{};
    SilKit_Struct_Init(SilKit_EthernetFrame, ef);
    ef.raw = {buffer.data(), buffer.size()};

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

    EXPECT_CALL(mockController, AddFrameHandler(testing::_, static_cast<SilKit::Services::DirectionMask>(
                                                                SilKit::Services::TransmitDirection::RX)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL,
                                                           &FrameHandler, SilKit_Direction_Receive, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHandler(testing::_, static_cast<SilKit::Services::DirectionMask>(
                                                                SilKit::Services::TransmitDirection::TX)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL,
                                                           &FrameHandler, SilKit_Direction_Send, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHandler(testing::_, static_cast<SilKit::Services::DirectionMask>(
                                                                SilKit::Services::TransmitDirection::TXRX)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL,
                                                           &FrameHandler, SilKit_Direction_SendReceive, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_RemoveFrameHandler((SilKit_EthernetController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, static_cast<EthernetTransmitStatusMask>(
                                                                        EthernetTransmitStatus::Transmitted)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        (SilKit_EthernetController*)&mockController, NULL, &FrameTransmitHandler,
        SilKit_EthernetTransmitStatus_Transmitted, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(
        mockController,
        AddFrameTransmitHandler(testing::_, static_cast<EthernetTransmitStatusMask>(EthernetTransmitStatus::LinkDown)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                   &FrameTransmitHandler,
                                                                   SilKit_EthernetTransmitStatus_LinkDown, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(
        mockController,
        AddFrameTransmitHandler(testing::_, static_cast<EthernetTransmitStatusMask>(EthernetTransmitStatus::Dropped)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                   &FrameTransmitHandler,
                                                                   SilKit_EthernetTransmitStatus_Dropped, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, static_cast<EthernetTransmitStatusMask>(
                                                                        EthernetTransmitStatus::InvalidFrameFormat)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        (SilKit_EthernetController*)&mockController, NULL, &FrameTransmitHandler,
        SilKit_EthernetTransmitStatus_InvalidFrameFormat, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, static_cast<EthernetTransmitStatusMask>(
                                                                        EthernetTransmitStatus::ControllerInactive)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        (SilKit_EthernetController*)&mockController, NULL, &FrameTransmitHandler,
        SilKit_EthernetTransmitStatus_ControllerInactive, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController,
                AddFrameTransmitHandler(
                    testing::_, static_cast<EthernetTransmitStatusMask>(EthernetTransmitStatus::InvalidFrameFormat)
                                    | static_cast<EthernetTransmitStatusMask>(EthernetTransmitStatus::Transmitted)))
        .Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        (SilKit_EthernetController*)&mockController, NULL, &FrameTransmitHandler,
        SilKit_EthernetTransmitStatus_InvalidFrameFormat | SilKit_EthernetTransmitStatus_Transmitted, &handlerId);
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

    WireEthernetFrame referenceFrame{};
    referenceFrame.raw = SilKit::Util::SharedVector<uint8_t>{SilKit::Util::MakeSpan(buffer)};

    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(ToEthernetFrame(referenceFrame)), nullptr)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, NULL);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    const auto userContext = reinterpret_cast<void*>(0x12345);

    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(ToEthernetFrame(referenceFrame)), userContext)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, userContext);
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

    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL, nullptr,
                                                           SilKit_Direction_SendReceive, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameHandler(nullptr, NULL, &FrameHandler, SilKit_Direction_SendReceive,
                                                           &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameHandler((SilKit_EthernetController*)&mockController, NULL,
                                                           &FrameHandler, SilKit_Direction_SendReceive, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL, nullptr,
                                                          SilKit_EthernetTransmitStatus_Transmitted, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        nullptr, NULL, &FrameTransmitHandler, SilKit_EthernetTransmitStatus_Transmitted, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_EthernetController_AddFrameTransmitHandler((SilKit_EthernetController*)&mockController, NULL,
                                                                   &FrameTransmitHandler,
                                                                   SilKit_EthernetTransmitStatus_Transmitted, nullptr);
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

    const auto testUserContext = reinterpret_cast<void *>(0x12345);

    SilKit_EthernetFrame ef{};
    SilKit_Struct_Init(SilKit_EthernetFrame, ef);

    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, nullptr, testUserContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_SendFrame(nullptr, &ef, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_EthernetController_SendFrame(nullptr, &ef, testUserContext);
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

    SilKit_EthernetFrame ef{};
    SilKit_Struct_Init(SilKit_EthernetFrame, ef);
    ef.raw = {(const uint8_t*)buffer, PAYLOAD_OFFSET + payloadSize};

    const auto testUserContext = reinterpret_cast<void *>(0x12345);

    EthernetFrame refFrame{};
    std::vector<uint8_t> rawFrame(ef.raw.data, ef.raw.data + ef.raw.size);
    refFrame.raw = rawFrame;
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(refFrame), testUserContext)).Times(testing::Exactly(1));
    returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, testUserContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

}

TEST_F(CapiEthernetTest, ethernet_controller_send_short_frame)
{
    std::vector<uint8_t> buffer;

    // set destination mac
    const EthernetMac destinationMac{ 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1 };
    std::copy(destinationMac.begin(), destinationMac.end(), std::back_inserter(buffer));

    // set source mac
    const EthernetMac sourceMac{ 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2 };
    std::copy(sourceMac.begin(), sourceMac.end(), std::back_inserter(buffer));

    // set ethertype
    buffer.push_back(0x00);
    buffer.push_back(0x08);

    // set payload
    const auto payload = std::string{"SHORT"};
    std::copy(payload.begin(), payload.end(), std::back_inserter(buffer));

    // create the Ethernet frame being sent
    SilKit_EthernetFrame ef{};
    SilKit_Struct_Init(SilKit_EthernetFrame, ef);
    ef.raw = {buffer.data(), buffer.size()};

    const auto testUserContext = reinterpret_cast<void *>(0x12345);

    // create the expected Ethernet frame
    std::vector<uint8_t> expectedFrameRaw(ef.raw.data, ef.raw.data + ef.raw.size);
    EthernetFrame expectedFrame{};
    expectedFrame.raw = expectedFrameRaw;

    // NOTE: The C API just hands the raw Ethernet frame to the underlying implementation. Any padding would be handled
    //       in the implementation.
    EXPECT_CALL(mockController, SendFrame(EthFrameMatcher(expectedFrame), testUserContext)).Times(testing::Exactly(1));

    const auto returnCode = SilKit_EthernetController_SendFrame((SilKit_EthernetController*)&mockController, &ef, testUserContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

}
