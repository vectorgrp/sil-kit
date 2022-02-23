/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/data/all.hpp"
#include "MockComAdapter.hpp"

namespace {
using namespace ib::sim::data;
using ib::mw::test::DummyComAdapter;

MATCHER_P(PayloadMatcher, controlPayload, "matches data payloads by their content and length")
{
    if (arg.size() != controlPayload.size())
    {
        return false;
    }
    for (int i = 0; i < arg.size(); i++)
    {
        if (arg[i] != controlPayload[i])
        {
            return false;
        }
    }
    return true;
}

class MockDataPublisher : public ib::sim::data::IDataPublisher
{
public:
    MOCK_METHOD1(Publish, void(std::vector<uint8_t> data));
    virtual void Publish(const uint8_t* data, std::size_t size){};
};
class MockDataSubscriber : public ib::sim::data::IDataSubscriber
{
public:
    MOCK_METHOD1(SetDefaultReceiveMessageHandler, void(DataHandlerT callback));
    MOCK_METHOD((void), RegisterSpecificDataHandler,
                (const DataExchangeFormat& dataExchangeFormat, (const std::map<std::string, std::string>& labels),
                 (DataHandlerT callback)),
                (override));
};

class MockComAdapter : public ib::mw::test::DummyComAdapter
{
public:
    MOCK_METHOD(ib::sim::data::IDataPublisher*, CreateDataPublisher,
                (const std::string& /*topic*/, const ib::sim::data::DataExchangeFormat& /*dataExchangeFormat*/,
                 (const std::map<std::string, std::string>& /*labels*/), size_t /* history */),
                (override));

    MOCK_METHOD(ib::sim::data::IDataSubscriber*, CreateDataSubscriber,
                (const std::string& /*topic*/, const ib::sim::data::DataExchangeFormat& /*dataExchangeFormat*/,
                 (const std::map<std::string, std::string>& /*labels*/), (ib::sim::data::DataHandlerT /* callback*/),
                 (ib::sim::data::NewDataSourceHandlerT /* callback*/)),
                (override));
};

void Copy_Label(ib_KeyValuePair* dst, const ib_KeyValuePair* src)
{
    auto lenKey = strlen(src->key) + 1;
    auto lenVal = strlen(src->value) + 1;
    dst->key = (const char*)malloc(lenKey);
    dst->value = (const char*)malloc(lenVal);
    if (dst->key != nullptr && dst->value != nullptr)
    {
        strcpy((char*)dst->key, src->key);
        strcpy((char*)dst->value, src->value);
    }
}

void Create_Labels(ib_KeyValueList** outLabels, const ib_KeyValuePair* labels, uint32_t numLabels)
{
    ib_KeyValueList* newLabels;
    size_t labelsSize = numLabels * sizeof(ib_KeyValuePair);
    size_t labelListSize = sizeof(ib_KeyValueList) + labelsSize;
    newLabels = (ib_KeyValueList*)malloc(labelListSize);
    if (newLabels != nullptr)
    {
        newLabels->numLabels = numLabels;
        for (uint32_t i = 0; i < numLabels; i++)
        {
            Copy_Label(&newLabels->labels[i], &labels[i]);
        }
    }
    *outLabels = newLabels;
}

class CapiDataTest : public testing::Test
{
public:
    MockDataPublisher mockDataPublisher;
    MockDataSubscriber mockDataSubscriber;
    MockComAdapter mockSimulationParticipant;

    CapiDataTest()
    {
        uint32_t numLabels = 1;
        ib_KeyValuePair labels[1] = {{"KeyA", "ValA"}};
        Create_Labels(&labelList, labels, numLabels);

        dataExchangeFormat.mediaType = "A";

        dummyContext.someInt = 1234;
        dummyContextPtr = (void*)&dummyContext;
    }
    ~CapiDataTest() { free(labelList); }

    typedef struct
    {
        uint32_t someInt;
    } TransmitContext;

    TransmitContext dummyContext;
    void* dummyContextPtr;

    ib_Data_ExchangeFormat dataExchangeFormat;
    ib_KeyValueList* labelList;
    uint32_t numLabels = 1;
};

void DefaultDataHandler(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data)
{
}

void NewDataSourceHandler(void* context, ib_Data_Subscriber* subscriber, const char* topic,
                          const ib_Data_ExchangeFormat* dataExchangeFormat, const ib_KeyValueList* labelList)
{
}

TEST_F(CapiDataTest, data_publisher_function_mapping)
{
    ib_ReturnCode returnCode;
    ib_Data_Publisher* publisher;

    EXPECT_CALL(mockSimulationParticipant, CreateDataPublisher).Times(testing::Exactly(1));
    returnCode = ib_Data_Publisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                          &dataExchangeFormat, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    ib_ByteVector data = {0, 0};
    EXPECT_CALL(mockDataPublisher, Publish(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Data_Publisher_Publish((ib_Data_Publisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_subscriber_function_mapping)
{
    ib_ReturnCode returnCode;

    ib_Data_Subscriber* subscriber;

    EXPECT_CALL(mockSimulationParticipant, CreateDataSubscriber).Times(testing::Exactly(1));
    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                           &dataExchangeFormat, labelList, nullptr, &DefaultDataHandler,
                                           nullptr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, SetDefaultReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Data_Subscriber_SetDefaultReceiveDataHandler((ib_Data_Subscriber*)&mockDataSubscriber, nullptr,
                                                          &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, RegisterSpecificDataHandler(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode =
        ib_Data_Subscriber_RegisterSpecificDataHandler((ib_Data_Subscriber*)&mockDataSubscriber, &dataExchangeFormat,
                                                       labelList, nullptr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_publisher_bad_parameters)
{
    ib_ByteVector data = {0, 0};
    ib_ReturnCode returnCode;
    ib_Data_Publisher* publisher;

    returnCode = ib_Data_Publisher_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                          &dataExchangeFormat, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, nullptr, "topic", &dataExchangeFormat, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr,
                                          &dataExchangeFormat, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                          nullptr, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Publish(nullptr, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Publish((ib_Data_Publisher*)&mockDataPublisher, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}

TEST_F(CapiDataTest, data_subscriber_bad_parameters)
{
    ib_ReturnCode returnCode;
    ib_Data_Subscriber* subscriber;

    returnCode = ib_Data_Subscriber_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                           &dataExchangeFormat, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode =
        ib_Data_Subscriber_Create(&subscriber, nullptr, "topic", &dataExchangeFormat, labelList,
                                  dummyContextPtr, &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr,
                                           &dataExchangeFormat, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                           nullptr, labelList, dummyContextPtr, &DefaultDataHandler,
                                           dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_SimulationParticipant*)&mockSimulationParticipant, "topic",
                                           &dataExchangeFormat, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_SetDefaultReceiveDataHandler(nullptr, dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode =
        ib_Data_Subscriber_SetDefaultReceiveDataHandler((ib_Data_Subscriber*)&mockDataSubscriber, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_RegisterSpecificDataHandler(nullptr, &dataExchangeFormat, labelList,
                                                                dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_RegisterSpecificDataHandler(
        (ib_Data_Subscriber*)&mockDataSubscriber, nullptr, labelList, dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_RegisterSpecificDataHandler(
        (ib_Data_Subscriber*)&mockDataSubscriber, &dataExchangeFormat, labelList, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}

TEST_F(CapiDataTest, data_publisher_publish)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "PUBSUB %i", messageCounter);
    ib_ByteVector data = {&buffer[0], payloadSize};

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockDataPublisher, Publish(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = ib_Data_Publisher_Publish((ib_Data_Publisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

} // namespace
