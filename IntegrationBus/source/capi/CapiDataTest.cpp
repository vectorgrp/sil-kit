/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/data/all.hpp"
#include "ib/cfg/Config.hpp"
#include "MockComAdapter.hpp"


namespace {
using namespace ib::sim::data;
using ib::mw::test::DummyComAdapter;

MATCHER_P(PayloadMatcher, controlPayload, "matches data payloads by their content and length") {
    if (arg.size() != controlPayload.size()) {
        return false;
    }
    for (int i = 0; i < arg.size(); i++) {
        if (arg[i] != controlPayload[i]) {
            return false;
        }
    }
    return true;
}

class MockDataPublisher : public ib::sim::data::IDataPublisher {
public:
    auto Config() const -> const ib::cfg::DataPort&
    {
        static const ib::cfg::DataPort portCfg{};
        return portCfg;
    };
    MOCK_METHOD1(Publish, void(std::vector<uint8_t> data));
    virtual void Publish(const uint8_t* data, std::size_t size) {};
};
class MockDataSubscriber : public ib::sim::data::IDataSubscriber{
public:
    auto Config() const -> const ib::cfg::DataPort&
    {
        static const ib::cfg::DataPort portCfg{};
        return portCfg;
    };
    MOCK_METHOD1(SetReceiveMessageHandler, void(std::function<void(IDataSubscriber* subscriber, const std::vector<uint8_t>& data, const ib::sim::data::DataExchangeFormat& dataExchangeFormat)> callback));
};

class MockComAdapter : public ib::mw::test::DummyComAdapter
{
  public:
    MOCK_METHOD(ib::sim::data::IDataPublisher*, CreateDataPublisher,
                (const std::string& /*canonicalName*/, 
                 const ib::sim::data::DataExchangeFormat& /*dataExchangeFormat*/,
                 size_t /* history */));
    MOCK_METHOD(ib::sim::data::IDataSubscriber*, CreateDataSubscriber,
                (const std::string& /*canonicalName*/, 
                 const ib::sim::data::DataExchangeFormat& /*dataExchangeFormat*/,
                 std::function<void(ib::sim::data::IDataSubscriber* subscriber, 
                                    const std::vector<uint8_t>& data,
                                    const ib::sim::data::DataExchangeFormat& dataExchangeFormat)> /* callback*/));

};

class CapiDataTest : public testing::Test
{
public: 
    MockDataPublisher mockDataPublisher;
    MockDataSubscriber mockDataSubscriber;
    MockComAdapter mockSimulationParticipant;
    CapiDataTest()
    {
    }
};

void ReceiveMessage(void* context, ib_DataSubscriber* subscriber, const ib_ByteVector* data, const ib_DataExchangeFormat* dataExchangeFormat) {

}

TEST_F(CapiDataTest, data_publisher_function_mapping)
{

    ib_ReturnCode returnCode;

    ib_DataPublisher* publisher;
    ib_DataExchangeFormat dataExchangeFormat;
    dataExchangeFormat.mediaType = "A";
    EXPECT_CALL(mockSimulationParticipant, CreateDataPublisher).Times(testing::Exactly(1));
    returnCode = ib_DataPublisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", &dataExchangeFormat, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    ib_ByteVector data = { 0,0 };
    EXPECT_CALL(mockDataPublisher, Publish(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_DataPublisher_Publish((ib_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_subscriber_function_mapping)
{
    ib_ReturnCode returnCode;

    ib_DataSubscriber* subscriber;
    ib_DataExchangeFormat dataExchangeFormat;
    dataExchangeFormat.mediaType = "A";
    EXPECT_CALL(mockSimulationParticipant, CreateDataSubscriber).Times(testing::Exactly(1));
    returnCode = ib_DataSubscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", &dataExchangeFormat, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, SetReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_DataSubscriber_SetReceiveDataHandler((ib_DataSubscriber*)&mockDataSubscriber, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_publisher_bad_parameters)
{
    ib_ByteVector data = { 0,0 };
    ib_ReturnCode returnCode;
    ib_DataExchangeFormat dataExchangeFormat;
    dataExchangeFormat.mediaType = "A";
    ib_DataPublisher* publisher;

    returnCode = ib_DataPublisher_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", &dataExchangeFormat, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_Create(&publisher, nullptr, "topic", &dataExchangeFormat, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr, &dataExchangeFormat, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", nullptr, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_DataPublisher_Publish(nullptr, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_Publish((ib_DataPublisher*)&mockDataPublisher, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}

TEST_F(CapiDataTest, data_subscriber_bad_parameters)
{
    ib_ReturnCode returnCode;
    ib_DataExchangeFormat dataExchangeFormat;
    dataExchangeFormat.mediaType = "A";
    ib_DataSubscriber* subscriber;

    returnCode = ib_DataSubscriber_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", &dataExchangeFormat, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataSubscriber_Create(&subscriber, nullptr, "topic", &dataExchangeFormat, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataSubscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr, &dataExchangeFormat, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataSubscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", nullptr, nullptr, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataSubscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", &dataExchangeFormat, nullptr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}


TEST_F(CapiDataTest, data_publisher_publish)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "PUBSUB %i", messageCounter);
    ib_ByteVector data = { &buffer[0], payloadSize };

    std::vector<uint8_t> refData(&(data.pointer[0]), &(data.pointer[0]) + data.size);
    EXPECT_CALL(mockDataPublisher, Publish(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = ib_DataPublisher_Publish((ib_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

}
