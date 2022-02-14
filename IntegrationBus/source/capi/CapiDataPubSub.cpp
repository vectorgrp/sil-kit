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
                                      const char* topic, ib_Data_ExchangeFormat* dataExchangeFormat,
                                      const ib_KeyValueList* labels, uint8_t history)
{
    ASSERT_VALID_OUT_PARAMETER(outPublisher);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_POINTER_PARAMETER(dataExchangeFormat);
    CAPI_ENTER
    {
        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        ib::sim::data::DataExchangeFormat cppDataTypeInfo{ dataExchangeFormat->mediaType };
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto dataPublisher = comAdapter->CreateDataPublisher(strTopic, cppDataTypeInfo, cppLabels, history);
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
                                       const char* topic, ib_Data_ExchangeFormat* dataExchangeFormat,
                                       const ib_KeyValueList* labels,
                                       void* defaultDataHandlerContext, ib_Data_Handler_t defaultDataHandler,
                                       void* newDataSourceContext, ib_Data_NewDataSourceHandler_t newDataSourceHandler)
{
    ASSERT_VALID_OUT_PARAMETER(outSubscriber);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(topic);
    ASSERT_VALID_HANDLER_PARAMETER(defaultDataHandler);
    ASSERT_VALID_POINTER_PARAMETER(dataExchangeFormat);
    ASSERT_VALID_HANDLER_PARAMETER(newDataSourceHandler);
    CAPI_ENTER
    {
        std::string strTopic(topic);
        auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        ib::sim::data::DataExchangeFormat cppDataTypeInfo{ dataExchangeFormat->mediaType };
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);

        auto cppDefaultDataHandler = [defaultDataHandler, defaultDataHandlerContext, outSubscriber](
                                         ib::sim::data::IDataSubscriber* cppSubscriber,
                                         const std::vector<uint8_t>& data) {
            uint8_t* payloadPointer = NULL;
            if (data.size() > 0)
            {
                payloadPointer = (uint8_t* const) & (data[0]);
            }
            const ib_ByteVector ccdata{payloadPointer, data.size()};
            defaultDataHandler(defaultDataHandlerContext, *outSubscriber, &ccdata);
        };

        auto cppNewDataSourceHandler = [newDataSourceHandler, newDataSourceContext, outSubscriber](
                                           ib::sim::data::IDataSubscriber* subscriber, const std::string& cppTopic,
                                           const ib::sim::data::DataExchangeFormat& cppDataExchangeFormat,
                                           const std::map<std::string, std::string>& cppLabels) {
            ib_Data_ExchangeFormat dxf;
            dxf.mediaType = cppDataExchangeFormat.mediaType.c_str();
            ib_KeyValueList* labels;
            assign(&labels, cppLabels);
            newDataSourceHandler(newDataSourceContext, *outSubscriber, cppTopic.c_str(), &dxf, labels);
        };

        auto dataSubscriber = comAdapter->CreateDataSubscriber(strTopic, cppDataTypeInfo, cppLabels,
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
            [dataHandler, context, self](ib::sim::data::IDataSubscriber* cppSubscriber,
                                         const std::vector<uint8_t>& data) {
                uint8_t* payloadPointer = NULL;
                if (data.size() > 0)
                {
                    payloadPointer = (uint8_t* const) & (data[0]);
                }
                const ib_ByteVector ccdata{payloadPointer, data.size()};
                dataHandler(context, self, &ccdata);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(ib_Data_Subscriber* self,
                                                            ib_Data_ExchangeFormat* dataExchangeFormat,
                                                            const ib_KeyValueList* labels,
                                                            void* context, ib_Data_Handler_t dataHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(dataExchangeFormat);
    ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
    CAPI_ENTER
    {
        auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(self);
        ib::sim::data::DataExchangeFormat cppDataTypeInfo{dataExchangeFormat->mediaType};
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppSubscriber->RegisterSpecificDataHandler(cppDataTypeInfo, cppLabels,
            [dataHandler, context, self](ib::sim::data::IDataSubscriber* cppSubscriber,
                                         const std::vector<uint8_t>& data) {
                uint8_t* payloadPointer = NULL;
                if (data.size() > 0)
                {
                    payloadPointer = (uint8_t* const) & (data[0]);
                }
                const ib_ByteVector ccdata{payloadPointer, data.size()};
                dataHandler(context, self, &ccdata);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}

