#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/data/all.hpp"

#include "CapiImpl.h"
#include "TypeConversion.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

extern "C" {

ib_ReturnCode ib_Data_Publisher_Create(ib_Data_Publisher** outPublisher, ib_Participant* participant,
                                           const char* controllerName, const char* topic,
                                           const char* mediaType, const ib_KeyValueList* labels,
                                           uint8_t history)
    {
    ASSERT_VALID_OUT_PARAMETER(outPublisher);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto dataPublisher = cppParticipant->CreateDataPublisher(controllerName, topic, mediaType, cppLabels, history);
        *outPublisher = reinterpret_cast<ib_Data_Publisher*>(dataPublisher);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Publisher_Publish(ib_Data_Publisher* self, const ib_ByteVector* data)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(data);
    CAPI_ENTER
    {
        auto cppPublisher = reinterpret_cast<ib::sim::data::IDataPublisher*>(self);
        cppPublisher->Publish(std::vector<uint8_t>(&(data->data[0]), &(data->data[0]) + data->size));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_Create(ib_Data_Subscriber** outSubscriber, ib_Participant* participant,
                                        const char* controllerName, const char* topic, const char* mediaType,
                                        const ib_KeyValueList* labels, void* defaultDataHandlerContext,
                                        ib_Data_DataMessageHandler_t defaultDataHandler, void* newDataSourceContext,
                                        ib_Data_NewDataPublisherHandler_t newDataSourceHandler)
{
    ASSERT_VALID_OUT_PARAMETER(outSubscriber);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(newDataSourceHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);

        auto cppDefaultDataHandler = [defaultDataHandler, defaultDataHandlerContext](
                                         ib::sim::data::IDataSubscriber* cppSubscriber,
                                         const ib::sim::data::DataMessageEvent& cppDataMessageEvent) {
            auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriber);
            uint8_t* payloadPointer = NULL;
            if (cppDataMessageEvent.data.size() > 0)
            {
                payloadPointer = (uint8_t*) &(cppDataMessageEvent.data[0]);
            }

            ib_Data_DataMessageEvent cDataMessageEvent;
            cDataMessageEvent.interfaceId = ib_InterfaceIdentifier_DataMessageEvent;
            cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
            cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };
            
            defaultDataHandler(defaultDataHandlerContext, cSubscriber, &cDataMessageEvent);
        };

        auto cppNewDataSourceHandler = [newDataSourceHandler, newDataSourceContext](
                                           ib::sim::data::IDataSubscriber* cppSubscriber,
                                           const ib::sim::data::NewDataPublisherEvent& cppNewDataPublisherEvent) {
            auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriber);

            ib_KeyValueList* cLabels;
            assign(&cLabels, cppNewDataPublisherEvent.labels);

            ib_Data_NewDataPublisherEvent cNewDataPublisherEvent;
            cNewDataPublisherEvent.interfaceId = ib_InterfaceIdentifier_NewDataPublisherEvent;
            cNewDataPublisherEvent.timestamp = cppNewDataPublisherEvent.timestamp.count();
            cNewDataPublisherEvent.topic = cppNewDataPublisherEvent.topic.c_str();
            cNewDataPublisherEvent.mediaType = cppNewDataPublisherEvent.mediaType.c_str();
            cNewDataPublisherEvent.labels = cLabels;

            newDataSourceHandler(newDataSourceContext, cSubscriber, &cNewDataPublisherEvent);
        };

        auto dataSubscriber = cppParticipant->CreateDataSubscriber(controllerName, topic, mediaType, cppLabels,
                                                               cppDefaultDataHandler, cppNewDataSourceHandler);
        *outSubscriber = reinterpret_cast<ib_Data_Subscriber*>(dataSubscriber);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_SetDefaultDataMessageHandler(ib_Data_Subscriber* self, void* context,
                                                      ib_Data_DataMessageHandler_t dataHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(self);
    cppSubscriber->SetDefaultDataMessageHandler(
        [dataHandler, context](ib::sim::data::IDataSubscriber* cppSubscriberHandler,
            const ib::sim::data::DataMessageEvent& cppDataMessageEvent) {
                auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriberHandler);
                uint8_t* payloadPointer = NULL;
                if (cppDataMessageEvent.data.size() > 0)
                {
                    payloadPointer = (uint8_t*)&(cppDataMessageEvent.data[0]);
                }
                ib_Data_DataMessageEvent cDataMessageEvent;
                cDataMessageEvent.interfaceId = ib_InterfaceIdentifier_DataMessageEvent;
                cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
                cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };

                dataHandler(context, cSubscriber, &cDataMessageEvent);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_AddExplicitDataMessageHandler(ib_Data_Subscriber* self, void* context,
                                                               ib_Data_DataMessageHandler_t dataHandler,
                                                               const char* mediaType,
                                                               const ib_KeyValueList* labels)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(self);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppSubscriber->AddExplicitDataMessageHandler(
            [dataHandler, context](ib::sim::data::IDataSubscriber* cppSubscriberHandler,
                                         const ib::sim::data::DataMessageEvent& cppDataMessageEvent) {
                auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriberHandler);
                uint8_t* payloadPointer = NULL;
                if (cppDataMessageEvent.data.size() > 0)
                {
                    payloadPointer = (uint8_t* ) &(cppDataMessageEvent.data[0]);
                }
                ib_Data_DataMessageEvent cDataMessageEvent;
                cDataMessageEvent.interfaceId = ib_InterfaceIdentifier_DataMessageEvent;
                cDataMessageEvent.timestamp = cppDataMessageEvent.timestamp.count();
                cDataMessageEvent.data = { payloadPointer, cppDataMessageEvent.data.size() };

                dataHandler(context, cSubscriber, &cDataMessageEvent);
            }, mediaType, cppLabels);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}

