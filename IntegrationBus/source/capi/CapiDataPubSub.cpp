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

ib_ReturnCode ib_Data_Publisher_Create(ib_Data_Publisher** outPublisher, ib_SimulationParticipant* participant,
                                       const char* topic, const char* mediaType, const ib_KeyValueList* labels,
                                       uint8_t history)
{
    ASSERT_VALID_OUT_PARAMETER(outPublisher);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    CAPI_ENTER
    {
        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        std::string cppMediaType{ mediaType };
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto dataPublisher = comAdapter->CreateDataPublisher(strTopic, cppMediaType, cppLabels, history);
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

ib_ReturnCode ib_Data_Subscriber_Create(ib_Data_Subscriber** outSubscriber, ib_SimulationParticipant* participant,
                                       const char* topic, const char* mediaType,
                                       const ib_KeyValueList* labels,
                                       void* defaultDataHandlerContext, ib_Data_Handler_t defaultDataHandler,
                                       void* newDataSourceContext, ib_Data_NewDataSourceHandler_t newDataSourceHandler)
{
    ASSERT_VALID_OUT_PARAMETER(outSubscriber);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(newDataSourceHandler);
    CAPI_ENTER
    {
        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        std::string cppMediaType{ mediaType };
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);

        auto cppDefaultDataHandler = [defaultDataHandler, defaultDataHandlerContext](
                                         ib::sim::data::IDataSubscriber* cppSubscriber,
                                         const std::vector<uint8_t>& data) {
            auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriber);
            uint8_t* payloadPointer = NULL;
            if (data.size() > 0)
            {
                payloadPointer = (uint8_t*) &(data[0]);
            }
            const ib_ByteVector ccdata{payloadPointer, data.size()};
            defaultDataHandler(defaultDataHandlerContext, cSubscriber, &ccdata);
        };

        auto cppNewDataSourceHandler = [newDataSourceHandler, newDataSourceContext](
                                           ib::sim::data::IDataSubscriber* cppSubscriber, const std::string& cppTopic,
                                           const std::string& cppMediaType,
                                           const std::map<std::string, std::string>& cppLabelsHandler) {
            auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriber);
            const char* mediaType = cppMediaType.c_str();
            ib_KeyValueList* clabelsHandler;
            assign(&clabelsHandler, cppLabelsHandler);
            newDataSourceHandler(newDataSourceContext, cSubscriber, cppTopic.c_str(), mediaType, clabelsHandler);
        };

        auto dataSubscriber = comAdapter->CreateDataSubscriber(strTopic, cppMediaType, cppLabels,
                                                               cppDefaultDataHandler, cppNewDataSourceHandler);
        *outSubscriber = reinterpret_cast<ib_Data_Subscriber*>(dataSubscriber);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_SetDefaultReceiveDataHandler(ib_Data_Subscriber* self, void* context,
                                                              ib_Data_Handler_t dataHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(self);
        cppSubscriber->SetDefaultReceiveMessageHandler(
            [dataHandler, context](ib::sim::data::IDataSubscriber* cppSubscriberHandler,
                                         const std::vector<uint8_t>& data) {
                auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriberHandler);
                uint8_t* payloadPointer = NULL;
                if (data.size() > 0)
                {
                    payloadPointer = (uint8_t*) &(data[0]);
                }
                const ib_ByteVector ccdata{payloadPointer, data.size()};
                dataHandler(context, cSubscriber, &ccdata);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(ib_Data_Subscriber* self,
                                                             const char* mediaType,
                                                             const ib_KeyValueList* labels,
                                                             void* context, ib_Data_Handler_t dataHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(self);
        std::string cppMediaType{mediaType};
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppSubscriber->RegisterSpecificDataHandler(cppMediaType, cppLabels,
            [dataHandler, context](ib::sim::data::IDataSubscriber* cppSubscriberHandler,
                                         const std::vector<uint8_t>& data) {
                auto* cSubscriber = reinterpret_cast<ib_Data_Subscriber*>(cppSubscriberHandler);
                uint8_t* payloadPointer = NULL;
                if (data.size() > 0)
                {
                    payloadPointer = (uint8_t* ) &(data[0]);
                }
                const ib_ByteVector ccdata{payloadPointer, data.size()};
                dataHandler(context, cSubscriber, &ccdata);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}

