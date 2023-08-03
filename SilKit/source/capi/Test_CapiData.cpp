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
    MOCK_METHOD1(SetDataMessageHandler, void(DataMessageHandler callback));
};

class MockParticipant : public SilKit::Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(SilKit::Services::PubSub::IDataPublisher*, CreateDataPublisher,
                (const std::string& /*canonicalName*/, (const SilKit::Services::PubSub::PubSubSpec& /*dataSpec*/), size_t /*history*/),
                (override));

    MOCK_METHOD(SilKit::Services::PubSub::IDataSubscriber*, CreateDataSubscriber,
                (const std::string& /*canonicalName*/, (const SilKit::Services::PubSub::PubSubSpec& /*dataSpec*/),
                 (SilKit::Services::PubSub::DataMessageHandler /*dataMessageHandler*/)),
                (override));
};

void Copy_Label(SilKit_Label* dst, const SilKit_Label* src)
{
    dst->key = (const char*)malloc(strlen(src->key) + 1);
    dst->value = (const char*)malloc(strlen(src->value) + 1);

    strcpy((char*)dst->key, src->key);
    strcpy((char*)dst->value, src->value);
    dst->kind = src->kind;
}

void Create_Labels(SilKit_LabelList** outLabelList, const SilKit_Label* labels, size_t numLabels)
{
    SilKit_LabelList* newLabelList;
    newLabelList = (SilKit_LabelList*)malloc(sizeof(SilKit_LabelList));
    newLabelList->numLabels = numLabels;
    newLabelList->labels = (SilKit_Label*)malloc(numLabels * sizeof(SilKit_Label));

    for (size_t i = 0; i < numLabels; i++)
    {
        Copy_Label(&newLabelList->labels[i], &labels[i]);
    }
    *outLabelList = newLabelList;
}

class Test_CapiData : public testing::Test
{
public:
    MockDataPublisher mockDataPublisher;
    MockDataSubscriber mockDataSubscriber;
    MockParticipant mockParticipant;

    Test_CapiData()
    {
        uint32_t numLabels = 1;
        SilKit_Label labels[1] = {{"KeyA", "ValA", SilKit_LabelKind_Optional}};
        Create_Labels(&labelList, labels, numLabels);

        mediaType = "A";

        dummyContext.someInt = 1234;
        dummyContextPtr = (void*)&dummyContext;
    }

    ~Test_CapiData()
    {
        for (uint32_t index = 0; index != labelList->numLabels; ++index)
        {
            free(const_cast<char *>(labelList->labels[index].key));
            free(const_cast<char *>(labelList->labels[index].value));
        }
        free(labelList->labels);
        free(labelList);
    }

    typedef struct
    {
        uint32_t someInt;
    } TransmitContext;

    TransmitContext dummyContext;
    void* dummyContextPtr;

    const char* mediaType;
    SilKit_LabelList* labelList;
};

void SilKitCALL DefaultDataHandler(void* /*context*/, SilKit_DataSubscriber* /*subscriber*/, const SilKit_DataMessageEvent* /*dataMessageEvent*/)
{
}

TEST_F(Test_CapiData, data_publisher_function_mapping)
{
    SilKit_ReturnCode returnCode;
    SilKit_DataPublisher* publisher;

    SilKit_DataSpec dataSpec;
    SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
    dataSpec.topic = "TopicA";
    dataSpec.mediaType = "text/json";
    dataSpec.labelList.numLabels = 0;
    dataSpec.labelList.labels = nullptr;

    EXPECT_CALL(mockParticipant, CreateDataPublisher("publisher", testing::_, 0)).Times(testing::Exactly(1));
    returnCode =
        SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, "publisher", &dataSpec, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = {0, 0};
    EXPECT_CALL(mockDataPublisher, Publish(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataPublisher_Publish((SilKit_DataPublisher*)&mockDataPublisher, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiData, data_subscriber_function_mapping)
{
    SilKit_ReturnCode returnCode;

    SilKit_DataSubscriber* subscriber;

    EXPECT_CALL(mockParticipant,
                CreateDataSubscriber("subscriber", testing::_, testing::_))
        .Times(testing::Exactly(1));
    SilKit_DataSpec dataSpec;
    SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
    dataSpec.topic = "TopicA";
    dataSpec.mediaType = "text/json";
    dataSpec.labelList.numLabels = 0;
    dataSpec.labelList.labels = nullptr;
    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber",
                                              &dataSpec, nullptr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockDataSubscriber, SetDataMessageHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_DataSubscriber_SetDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber, nullptr,
                                                          &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

}

TEST_F(Test_CapiData, data_publisher_bad_parameters)
{
    SilKit_ByteVector data = {0, 0};
    SilKit_ReturnCode returnCode;
    SilKit_DataPublisher* publisher;
    SilKit_DataSpec dataSpec;
    SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
    dataSpec.topic = "TopicA";
    dataSpec.mediaType = "text/json";
    dataSpec.labelList.numLabels = 0;
    dataSpec.labelList.labels = nullptr;

    returnCode =
        SilKit_DataPublisher_Create(nullptr, (SilKit_Participant*)&mockParticipant, "publisher", &dataSpec, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, nullptr, "publisher", &dataSpec, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, nullptr, &dataSpec, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Create(&publisher, (SilKit_Participant*)&mockParticipant, "publisher", nullptr, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Publish(nullptr, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataPublisher_Publish((SilKit_DataPublisher*)&mockDataPublisher, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiData, data_subscriber_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_DataSubscriber* subscriber;
    SilKit_DataSpec dataSpec;
    SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
    dataSpec.topic = "TopicA";
    dataSpec.mediaType = "text/json";
    dataSpec.labelList.numLabels = 0;
    dataSpec.labelList.labels = nullptr;

    returnCode =
        SilKit_DataSubscriber_Create(nullptr, (SilKit_Participant*)&mockParticipant, "subscriber", &dataSpec,
                                     dummyContextPtr,
                                           &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_Create(&subscriber, nullptr, "subscriber", &dataSpec,
                                  dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, nullptr, &dataSpec,
                                  dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_Create(&subscriber, (SilKit_Participant*)&mockParticipant, "subscriber", nullptr, dummyContextPtr,
                                           &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_DataSubscriber_SetDataMessageHandler(nullptr, dummyContextPtr, &DefaultDataHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_DataSubscriber_SetDataMessageHandler((SilKit_DataSubscriber*)&mockDataSubscriber, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiData, data_publisher_publish)
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
