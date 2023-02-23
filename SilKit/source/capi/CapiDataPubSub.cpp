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

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <map>
#include <mutex>
#include <cstring>


SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher, SilKit_Participant* participant,
                                           const char* controllerName,
                                           SilKit_DataSpec* dataSpec,
                                           uint8_t history)
try
{
    ASSERT_VALID_OUT_PARAMETER(outPublisher);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(dataSpec);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    SilKit::Services::PubSub::PubSubSpec cppDataSpec;
    assign(cppDataSpec, dataSpec);

    auto dataPublisher = cppParticipant->CreateDataPublisher(controllerName, cppDataSpec, history);
    *outPublisher = reinterpret_cast<SilKit_DataPublisher*>(dataPublisher);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Publish(SilKit_DataPublisher* self, const SilKit_ByteVector* data)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(data);

    auto cppPublisher = reinterpret_cast<SilKit::Services::PubSub::IDataPublisher*>(self);
    cppPublisher->Publish(SilKit::Util::ToSpan(*data));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber, SilKit_Participant* participant,
                                               const char* controllerName, SilKit_DataSpec* dataSpec,
                                               void* defaultDataHandlerContext,
                                               SilKit_DataMessageHandler_t defaultDataHandler)
try
{
    ASSERT_VALID_OUT_PARAMETER(outSubscriber);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(dataSpec);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);

    SilKit::Services::PubSub::PubSubSpec cppDataNodeSpec;
    assign(cppDataNodeSpec, dataSpec);

    auto cppDefaultDataHandler = [defaultDataHandler, defaultDataHandlerContext](
                                     SilKit::Services::PubSub::IDataSubscriber* cppSubscriber,
                                     const SilKit::Services::PubSub::DataMessageEvent& cppDataMessageEvent) {
        auto* cSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(cppSubscriber);
        uint8_t* payloadPointer = nullptr;
        if (cppDataMessageEvent.data.size() > 0)
        {
            payloadPointer = (uint8_t*) &(cppDataMessageEvent.data[0]);
        }

        SilKit_DataMessageEvent cDataMessageEvent;
        SilKit_Struct_Init(SilKit_DataMessageEvent, cDataMessageEvent);
        cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
        cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };

        defaultDataHandler(defaultDataHandlerContext, cSubscriber, &cDataMessageEvent);
    };

    auto dataSubscriber = cppParticipant->CreateDataSubscriber(controllerName, cppDataNodeSpec,
                                                           cppDefaultDataHandler);
    *outSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(dataSubscriber);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_SetDataMessageHandler(SilKit_DataSubscriber* self, void* context,
                                                      SilKit_DataMessageHandler_t dataHandler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);

    auto cppSubscriber = reinterpret_cast<SilKit::Services::PubSub::IDataSubscriber*>(self);
    cppSubscriber->SetDataMessageHandler(
    [dataHandler, context](SilKit::Services::PubSub::IDataSubscriber* cppSubscriberHandler,
        const SilKit::Services::PubSub::DataMessageEvent& cppDataMessageEvent) {
            auto* cSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(cppSubscriberHandler);
            uint8_t* payloadPointer = nullptr;
            if (cppDataMessageEvent.data.size() > 0)
            {
                payloadPointer = (uint8_t*)&(cppDataMessageEvent.data[0]);
            }
            SilKit_DataMessageEvent cDataMessageEvent;
            SilKit_Struct_Init(SilKit_DataMessageEvent, cDataMessageEvent);
            cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
            cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };

            dataHandler(context, cSubscriber, &cDataMessageEvent);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
