// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

extern "C" {

SilKit_ReturnCode SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher, SilKit_Participant* participant,
                                           const char* controllerName, const char* topic,
                                           const char* mediaType, const SilKit_KeyValueList* labels,
                                           uint8_t history)
    {
    ASSERT_VALID_OUT_PARAMETER(outPublisher);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto dataPublisher = cppParticipant->CreateDataPublisher(controllerName, topic, mediaType, cppLabels, history);
        *outPublisher = reinterpret_cast<SilKit_DataPublisher*>(dataPublisher);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DataPublisher_Publish(SilKit_DataPublisher* self, const SilKit_ByteVector* data)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(data);
    CAPI_ENTER
    {
        auto cppPublisher = reinterpret_cast<SilKit::Services::PubSub::IDataPublisher*>(self);
        cppPublisher->Publish(SilKit::Util::ToSpan(*data));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber, SilKit_Participant* participant,
                                        const char* controllerName, const char* topic, const char* mediaType,
                                        const SilKit_KeyValueList* labels, void* defaultDataHandlerContext,
                                        SilKit_DataMessageHandler_t defaultDataHandler, void* newDataSourceContext,
                                        SilKit_NewDataPublisherHandler_t newDataSourceHandler)
{
    ASSERT_VALID_OUT_PARAMETER(outSubscriber);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(newDataSourceHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);

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

        auto cppNewDataSourceHandler = [newDataSourceHandler, newDataSourceContext](
                                           SilKit::Services::PubSub::IDataSubscriber* cppSubscriber,
                                           const SilKit::Services::PubSub::NewDataPublisherEvent& cppNewDataPublisherEvent) {
            auto* cSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(cppSubscriber);

            SilKit_KeyValueList* cLabels;
            assign(&cLabels, cppNewDataPublisherEvent.labels);

            SilKit_NewDataPublisherEvent cNewDataPublisherEvent;
            SilKit_Struct_Init(SilKit_NewDataPublisherEvent, cNewDataPublisherEvent);
            cNewDataPublisherEvent.timestamp = cppNewDataPublisherEvent.timestamp.count();
            cNewDataPublisherEvent.topic = cppNewDataPublisherEvent.topic.c_str();
            cNewDataPublisherEvent.mediaType = cppNewDataPublisherEvent.mediaType.c_str();
            cNewDataPublisherEvent.labels = cLabels;

            newDataSourceHandler(newDataSourceContext, cSubscriber, &cNewDataPublisherEvent);
        };

        auto dataSubscriber = cppParticipant->CreateDataSubscriber(controllerName, topic, mediaType, cppLabels,
                                                               cppDefaultDataHandler, cppNewDataSourceHandler);
        *outSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(dataSubscriber);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DataSubscriber_SetDefaultDataMessageHandler(SilKit_DataSubscriber* self, void* context,
                                                      SilKit_DataMessageHandler_t dataHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<SilKit::Services::PubSub::IDataSubscriber*>(self);
    cppSubscriber->SetDefaultDataMessageHandler(
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
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DataSubscriber_AddExplicitDataMessageHandler(SilKit_DataSubscriber* self, void* context,
                                                               SilKit_DataMessageHandler_t dataHandler,
                                                               const char* mediaType,
                                                               const SilKit_KeyValueList* labels,
                                                               SilKit_HandlerId * outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<SilKit::Services::PubSub::IDataSubscriber*>(self);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto cppHandlerId = cppSubscriber->AddExplicitDataMessageHandler(
            [dataHandler, context](SilKit::Services::PubSub::IDataSubscriber* cppSubscriberHandler,
                                         const SilKit::Services::PubSub::DataMessageEvent& cppDataMessageEvent) {
                auto* cSubscriber = reinterpret_cast<SilKit_DataSubscriber*>(cppSubscriberHandler);
                uint8_t* payloadPointer = nullptr;
                if (cppDataMessageEvent.data.size() > 0)
                {
                    payloadPointer = (uint8_t* ) &(cppDataMessageEvent.data[0]);
                }
                SilKit_DataMessageEvent cDataMessageEvent;
                SilKit_Struct_Init(SilKit_DataMessageEvent, cDataMessageEvent);
                cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
                cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };

                dataHandler(context, cSubscriber, &cDataMessageEvent);
            }, mediaType, cppLabels);
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DataSubscriber_RemoveExplicitDataMessageHandler(SilKit_DataSubscriber* self,
                                                                  SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<SilKit::Services::PubSub::IDataSubscriber*>(self);
        cppSubscriber->RemoveExplicitDataMessageHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}

