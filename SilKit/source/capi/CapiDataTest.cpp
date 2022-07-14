/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/pubsub/all.hpp"
#include "MockParticipant.hpp"

namespace {
using namespace SilKit::Services::PubSub;
using SilKit::Core::Tests::DummyParticipant;

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

class MockDataPublisher : public SilKit::Services::PubSub::IDataPublisher
{
public:
    MOCK_METHOD(void, Publish, (SilKit::Util::Span<const uint8_t> data), (override));
};

class MockDataSubscriber : public SilKit::Services::PubSub::IDataSubscriber
{
public:
    MOCK_METHOD1(SetDefaultDataMessageHandler, void(DataMessageHandlerT callback));
    MOCK_METHOD((SilKit::Util::HandlerId), AddExplicitDataMessageHandler,
                ((DataMessageHandlerT callback), const std::string& mediaType,
                 (const std::map<std::string, std::string>& labels)),
                (override));
    MOCK_METHOD(void, RemoveExplicitDataMessageHandler, (SilKit::Util::HandlerId handlerId), (override));
};

class MockParticipant : public SilKit::Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(SilKit::Services::PubSub::IDataPublisher*, CreateDataPublisher,
                (const std::string& /*controllerName*/, const std::string& /*topic*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/), size_t /* history */),
                (override));

    MOCK_METHOD(SilKit::Services::PubSub::IDataSubscriber*, CreateDataSubscriber,
                (const std::string& /*controllerName*/, const std::string& /*topic*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/), (SilKit::Services::PubSub::DataMessageHandlerT /* callback*/),
                 (SilKit::Services::PubSub::NewDataPublisherHandlerT /* callback*/)),
                (override));
};

void Copy_Label(SilKit_KeyValuePair* dst, const SilKit_KeyValuePair* src)
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

void Create_Labels(SilKit_KeyValueList** outLabels, const SilKit_KeyValuePair* labels, uint32_t numLabels)
{
    SilKit_KeyValueList* newLabels;
    newLabels = (SilKit_KeyValueList*)malloc(sizeof(SilKit_KeyValueList));
    if (newLabels == nullptr)
    {
        throw std::bad_alloc();
    }
    newLabels->numLabels = numLabels;
    newLabels->labels = (SilKit_KeyValuePair*)malloc(numLabels * sizeof(SilKit_KeyValuePair));
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
        SilKit_KeyValuePair labels[1] = {{"KeyA", "ValA"}};
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
    SilKit_KeyValueList* labelList;
};

void DefaultDataHandler(void* /*context*/, SilKit_DataSubscriber* /*subscriber*/, const SilKit_DataMessageEvent* /*dataMessageEvent*/)
{
}

void NewDataSourceHandler(void* /*context*/, SilKit_DataSubscriber* /*subscriber*/, const SilKit_NewDataPublisherEvent* /*newDataPublisherEvent*/)
{
}

TEST_F(CapiDataTest, data_publisher_function_mapping)
{
    SilKit_ReturnCode returnCode;
    SilKit_DataPublisher* publisher;

    EXPECT_CALL(mockParticipant, CreateDataPublisher("publisher", "topic", mediaType, testing::_, 0)).Times(testing::Exactly(1));
    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, "publisher", "topic",
                                          mediaType, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = {0, 0};
    EXPECT_CALL(mockDataPublisher, Publish(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataPublisher_Publish((SilKit_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_subscriber_function_mapping)
{
    SilKit_ReturnCode returnCode;

    SilKit_DataSubscriber* subscriber;

    EXPECT_CALL(mockParticipant, CreateDataSubscriber("subscriber", "topic", mediaType, testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, nullptr, &DefaultDataHandler,
                                           nullptr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, SetDefaultDataMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataSubscriber_SetDefaultDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber, nullptr,
                                                          &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_HandlerId handlerId;

    EXPECT_CALL(mockDataSubscriber, AddExplicitDataMessageHandler(testing::_, mediaType, testing::_))
        .Times(testing::Exactly(1));
    returnCode = SilKit_DataSubscriber_AddExplicitDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber, nullptr,
                                                                  &DefaultDataHandler, mediaType, labelList,
                                                                  &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, RemoveExplicitDataMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataSubscriber_RemoveExplicitDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber,
                                                                     handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiDataTest, data_publisher_bad_parameters)
{
    SilKit_ByteVector data = {0, 0};
    SilKit_ReturnCode returnCode;
    SilKit_DataPublisher* publisher;

    returnCode = SilKit_DataPublisher_Create(nullptr, (SilKit_Participant*)&mockParticipant, "publisher", "topic",
                                          mediaType, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, nullptr, "publisher", "topic", mediaType, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, nullptr,
                                          "topic", mediaType, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, "publisher", nullptr,
                                          mediaType, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, "publisher", "topic",
                                          nullptr, labelList, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Publish(nullptr, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Publish((SilKit_DataPublisher*)&mockDataPublisher, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(CapiDataTest, data_subscriber_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_DataSubscriber* subscriber;

    returnCode = SilKit_DataSubscriber_Create(nullptr, (SilKit_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_DataSubscriber_Create(&subscriber, nullptr, "subscriber", "topic", mediaType, labelList,
                                  dummyContextPtr, &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, nullptr, "topic", mediaType, labelList,
                                  dummyContextPtr, &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber", nullptr,
        mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber", "topic",
                                           nullptr, labelList, dummyContextPtr, &DefaultDataHandler,
                                           dummyContextPtr, &NewDataSourceHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber", "topic",
                                           mediaType, labelList, dummyContextPtr,
                                           &DefaultDataHandler, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_SetDefaultDataMessageHandler(nullptr, dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    constexpr SilKit_HandlerId defaultHanderId = 0xCAFECAFE;
    SilKit_HandlerId handlerId = defaultHanderId;

    returnCode =
        SilKit_DataSubscriber_SetDefaultDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    handlerId = defaultHanderId;
    returnCode = SilKit_DataSubscriber_AddExplicitDataMessageHandler(nullptr, dummyContextPtr, &DefaultDataHandler,
                                                                  mediaType, labelList, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(handlerId, defaultHanderId);

    handlerId = defaultHanderId;
    returnCode = SilKit_DataSubscriber_AddExplicitDataMessageHandler(
        (SilKit_DataSubscriber*)&mockDataSubscriber, dummyContextPtr, nullptr, mediaType, labelList, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(handlerId, defaultHanderId);

    handlerId = defaultHanderId;
    returnCode = SilKit_DataSubscriber_AddExplicitDataMessageHandler(
        (SilKit_DataSubscriber*)&mockDataSubscriber, dummyContextPtr, &DefaultDataHandler, nullptr, labelList, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(handlerId, defaultHanderId);

    handlerId = defaultHanderId;
    returnCode = SilKit_DataSubscriber_AddExplicitDataMessageHandler(
        (SilKit_DataSubscriber*)&mockDataSubscriber, dummyContextPtr, &DefaultDataHandler, mediaType, labelList, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    EXPECT_EQ(handlerId, defaultHanderId);

    returnCode = SilKit_DataSubscriber_RemoveExplicitDataMessageHandler(nullptr, (SilKit_HandlerId)0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(CapiDataTest, data_publisher_publish)
{
    SilKit_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "PUBSUB %i", messageCounter);
    SilKit_ByteVector data = {&buffer[0], payloadSize};

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockDataPublisher, Publish(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = SilKit_DataPublisher_Publish((SilKit_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} // namespace
