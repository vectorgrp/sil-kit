/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/data/all.hpp"
#include "MockParticipant.hpp"

namespace {
using namespace ib::sim::data;
using ib::mw::test::DummyParticipant;

MATCHER_P(PayloadMatcher, controlPayload, "")
{
    *result_listener << "matches data payloads by their content and length";
    if (arg.size() != controlPayload.size())
    {
        return false;
    }
    for (size_t i = 0; i < arg.size(); i++)
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
    virtual void Publish(const uint8_t* /*data*/, std::size_t /*size*/){};
};
class MockDataSubscriber : public ib::sim::data::IDataSubscriber
{
public:
    MOCK_METHOD1(SetDefaultDataMessageHandler, void(DataMessageHandlerT callback));
    MOCK_METHOD((void), AddExplicitDataMessageHandler,
                ((DataMessageHandlerT callback), const std::string& mediaType,
                 (const std::map<std::string, std::string>& labels)),
                (override));
};

class MockParticipant : public ib::mw::test::DummyParticipant
{
public:
    MOCK_METHOD(ib::sim::data::IDataPublisher*, CreateDataPublisher,
                (const std::string& /*controllerName*/, const std::string& /*topic*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/), size_t /* history */),
                (override));

    MOCK_METHOD(ib::sim::data::IDataSubscriber*, CreateDataSubscriber,
                (const std::string& /*controllerName*/, const std::string& /*topic*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/), (ib::sim::data::DataMessageHandlerT /* callback*/),
                 (ib::sim::data::NewDataPublisherHandlerT /* callback*/)),
                (override));
};

void Copy_Label(ib_KeyValuePair* dst, const ib_KeyValuePair* src)
{
    auto lenKey = strlen(src->key) + 1;
    auto lenVal = strlen(src->value) + 1;
    dst->key = (const char*)malloc(lenKey);
    dst->value = (const char*)malloc(lenVal);
    if (dst->key == nullptr || dst->value == nullptr)
    {
        throw std::bad_alloc();
    }
    strcpy((char*)dst->key, src->key);
    strcpy((char*)dst->value, src->value);
}

void Create_Labels(ib_KeyValueList** outLabels, const ib_KeyValuePair* labels, uint32_t numLabels)
{
    ib_KeyValueList* newLabels;
    newLabels = (ib_KeyValueList*)malloc(sizeof(ib_KeyValueList));
    if (newLabels == nullptr)
    {
        throw std::bad_alloc();
    }
    newLabels->numLabels = numLabels;
    newLabels->labels = (ib_KeyValuePair*)malloc(numLabels * sizeof(ib_KeyValuePair));
    if (newLabels->labels == nullptr)
    {
        throw std::bad_alloc();
    }
    for (uint32_t i = 0; i < numLabels; i++)
    {
        Copy_Label(&newLabels->labels[i], &labels[i]);
    }
    *outLabels = newLabels;
}

class CapiDataTest : public testing::Test
{
public:
    MockDataPublisher mockDataPublisher;
    MockDataSubscriber mockDataSubscriber;
    MockParticipant mockParticipant;

    CapiDataTest()
    {
        uint32_t numLabels = 1;
        ib_KeyValuePair labels[1] = {{"KeyA", "ValA"}};
        Create_Labels(&labelList, labels, numLabels);

        mediaType = "A";

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

    const char* mediaType;
    ib_KeyValueList* labelList;
};

void DefaultDataHandler(void* /*context*/, ib_Data_Subscriber* /*subscriber*/, const ib_Data_DataMessageEvent* /*dataMessageEvent*/)
{
}

void NewDataSourceHandler(void* /*context*/, ib_Data_Subscriber* /*subscriber*/, const ib_Data_NewDataPublisherEvent* /*newDataPublisherEvent*/)
{
}

TEST_F(CapiDataTest, data_publisher_function_mapping)
{
    ib_ReturnCode returnCode;
    ib_Data_Publisher* publisher;

    EXPECT_CALL(mockParticipant, CreateDataPublisher("publisher", "topic", mediaType, testing::_, 0)).Times(testing::Exactly(1));
    returnCode = ib_Data_Publisher_Create(&publisher, (ib_Participant*)&mockParticipant, "publisher", "topic",
                                          mediaType, labelList, 0);
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

    EXPECT_CALL(mockParticipant, CreateDataSubscriber("subscriber", "topic", mediaType, testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, nullptr, &DefaultDataHandler,
                                           nullptr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, SetDefaultDataMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Data_Subscriber_SetDefaultDataMessageHandler((ib_Data_Subscriber*)&mockDataSubscriber, nullptr,
                                                          &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, AddExplicitDataMessageHandler(testing::_, mediaType, testing::_))
        .Times(testing::Exactly(1));
    returnCode = ib_Data_Subscriber_AddExplicitDataMessageHandler((ib_Data_Subscriber*)&mockDataSubscriber, nullptr,
                                                                  &DefaultDataHandler, mediaType, labelList);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_publisher_bad_parameters)
{
    ib_ByteVector data = {0, 0};
    ib_ReturnCode returnCode;
    ib_Data_Publisher* publisher;

    returnCode = ib_Data_Publisher_Create(nullptr, (ib_Participant*)&mockParticipant, "publisher", "topic",
                                          mediaType, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, nullptr, "publisher", "topic", mediaType, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, (ib_Participant*)&mockParticipant, nullptr,
                                          "topic", mediaType, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, (ib_Participant*)&mockParticipant, "publisher", nullptr,
                                          mediaType, labelList, 0);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Publisher_Create(&publisher, (ib_Participant*)&mockParticipant, "publisher", "topic",
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

    returnCode = ib_Data_Subscriber_Create(nullptr, (ib_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode =
        ib_Data_Subscriber_Create(&subscriber, nullptr, "subscriber", "topic", mediaType, labelList,
                                  dummyContextPtr, &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode =
        ib_Data_Subscriber_Create(&subscriber, (ib_Participant*)&mockParticipant, nullptr, "topic", mediaType, labelList,
                                  dummyContextPtr, &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_Participant*)&mockParticipant, "subscriber", nullptr,
        mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_Participant*)&mockParticipant, "subscriber", "topic",
                                           nullptr, labelList, dummyContextPtr, &DefaultDataHandler,
                                           dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_Create(&subscriber, (ib_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_SetDefaultDataMessageHandler(nullptr, dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode =
        ib_Data_Subscriber_SetDefaultDataMessageHandler((ib_Data_Subscriber*)&mockDataSubscriber, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_AddExplicitDataMessageHandler(nullptr, dummyContextPtr, &DefaultDataHandler,
                                                                  mediaType, labelList);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_AddExplicitDataMessageHandler(
        (ib_Data_Subscriber*)&mockDataSubscriber, dummyContextPtr, nullptr, mediaType, labelList);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Data_Subscriber_AddExplicitDataMessageHandler(
        (ib_Data_Subscriber*)&mockDataSubscriber, dummyContextPtr, &DefaultDataHandler, nullptr, labelList);
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
