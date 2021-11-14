/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/generic/all.hpp"
#include "ib/cfg/Config.hpp"
#include "MockComAdapter.hpp"


namespace {
using namespace ib::sim::eth;
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

class MockDataPublisher : public ib::sim::generic::IGenericPublisher {
public:
    virtual auto Config() const -> const ib::cfg::GenericPort& {return {};};
    MOCK_METHOD1(Publish, void(std::vector<uint8_t> data));
    virtual void Publish(const uint8_t* data, std::size_t size) {};
};
class MockDataSubscriber : public ib::sim::generic::IGenericSubscriber{
public:
    virtual auto Config() const -> const ib::cfg::GenericPort& { return {}; };
    MOCK_METHOD1(SetReceiveMessageHandler, void(std::function<void(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)> callback));
};

class MockComAdapter : public ib::mw::test::DummyComAdapter
{
public:
    MOCK_METHOD1(CreateGenericPublisher, ib::sim::generic::IGenericPublisher* (const std::string&));
    MOCK_METHOD1(CreateGenericSubscriber, ib::sim::generic::IGenericSubscriber* (const std::string&));
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

void ReceiveMessage(void* context, ib_DataSubscriber* subscriber, const ib_ByteVector* data) {

}

TEST_F(CapiDataTest, data_publisher_function_mapping)
{

    ib_ReturnCode returnCode;

    ib_DataPublisher* publisher;
    EXPECT_CALL(mockSimulationParticipant, CreateGenericPublisher(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_DataPublisher_create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", NULL, 0);
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
    EXPECT_CALL(mockSimulationParticipant, CreateGenericSubscriber(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_DataSubscriber_create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", NULL, NULL, NULL);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    ib_ByteVector data = { 0,0 };
    EXPECT_CALL(mockDataSubscriber, SetReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_DataSubscriber_SetReceiveDataHandler((ib_DataSubscriber*)&mockDataSubscriber, NULL, &ReceiveMessage);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_publisher_bad_parameters)
{
    ib_ByteVector data = { 0,0 };
    ib_ReturnCode returnCode;

    ib_DataPublisher* publisher;
    returnCode = ib_DataPublisher_create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic", NULL, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_create(&publisher, nullptr, "topic", NULL, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr, NULL, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_DataPublisher_Publish(nullptr, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_DataPublisher_Publish((ib_DataPublisher*)&mockDataPublisher, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}


TEST_F(CapiDataTest, data_publisher_publish)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "ETHERNET %i", messageCounter);
    ib_ByteVector data = { &buffer[0], payloadSize };

    std::vector<uint8_t> refData(&(data.pointer[0]), &(data.pointer[0]) + data.size);
    EXPECT_CALL(mockDataPublisher, Publish(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = ib_DataPublisher_Publish((ib_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

}