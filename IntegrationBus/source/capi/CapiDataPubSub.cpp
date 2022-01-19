#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/data/all.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.h"

IntegrationBusAPI ib_ReturnCode ib_DataPublisher_Create(
  ib_DataPublisher** out,
  ib_SimulationParticipant* participant,
  const char* topic,
  ib_DataExchangeFormat* dataTypeInfo,
  uint8_t history)
{
  ASSERT_VALID_OUT_PARAMETER(out);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(topic);
  ASSERT_VALID_POINTER_PARAMETER(dataTypeInfo);
  CAPI_ENTER
  {
    std::string strTopic(topic);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    ib::sim::data::DataExchangeFormat cppDataTypeInfo{std::string(dataTypeInfo->mediaType)};
    auto dataPublisher = comAdapter->CreateDataPublisher(strTopic, cppDataTypeInfo, history);
    *out = reinterpret_cast<ib_DataPublisher*>(dataPublisher);
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}


IntegrationBusAPI ib_ReturnCode ib_DataSubscriber_Create(
  ib_DataSubscriber** out,
  ib_SimulationParticipant* participant,
  const char* topic,
  ib_DataExchangeFormat* dataTypeInfo,
  void* context,
  ib_DataHandler_t dataHandler)
{
  ASSERT_VALID_OUT_PARAMETER(out);
  ASSERT_VALID_POINTER_PARAMETER(participant);
  ASSERT_VALID_POINTER_PARAMETER(topic);
  ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
  ASSERT_VALID_POINTER_PARAMETER(dataTypeInfo);
  CAPI_ENTER
  {
    std::string strTopic(topic);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    ib::sim::data::DataExchangeFormat cppDataTypeInfo{ std::string(dataTypeInfo->mediaType) };
    auto dataSubscriber = comAdapter->CreateDataSubscriber(strTopic, cppDataTypeInfo,
        [dataHandler, context, out](ib::sim::data::IDataSubscriber* cppSubscriber, const std::vector<uint8_t>& data, const ib::sim::data::DataExchangeFormat& joinedDataExchangeFormat) {
            uint8_t* payloadPointer = NULL;
            if (data.size() > 0)
            {
                payloadPointer = (uint8_t* const) & (data[0]);
            }
            const ib_ByteVector ccdata{payloadPointer, data.size()};
            ib_DataExchangeFormat jdxf;
            jdxf.mediaType = joinedDataExchangeFormat.mimeType.c_str();
            dataHandler(context, *out, &ccdata, &jdxf);
        });

    *out = reinterpret_cast<ib_DataSubscriber*>(dataSubscriber);


    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_DataSubscriber_SetReceiveDataHandler(
  ib_DataSubscriber* subscriber,
  void* context,
  ib_DataHandler_t dataHandler)
{
  ASSERT_VALID_POINTER_PARAMETER(subscriber);
  ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
  CAPI_ENTER
  {
    auto cppSubscriber = reinterpret_cast<ib::sim::data::IDataSubscriber*>(subscriber);
    cppSubscriber->SetReceiveMessageHandler(
      [dataHandler, context, subscriber](ib::sim::data::IDataSubscriber* cppSubscriber, const std::vector<uint8_t>& data, const ib::sim::data::DataExchangeFormat& joinedDataExchangeFormat)
      {
        uint8_t* payloadPointer = NULL;
        if (data.size() > 0)
        {
          payloadPointer = (uint8_t* const)&(data[0]);
        }
        const ib_ByteVector ccdata{ payloadPointer, data.size() };
        ib_DataExchangeFormat jdxf;
        jdxf.mediaType = joinedDataExchangeFormat.mimeType.c_str();
        dataHandler(context, subscriber,  &ccdata, &jdxf);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_DataPublisher_Publish(ib_DataPublisher* publisher, const ib_ByteVector* data)
{
  ASSERT_VALID_POINTER_PARAMETER(publisher);
  ASSERT_VALID_POINTER_PARAMETER(data);
  CAPI_ENTER
  {
    auto cppPublisher = reinterpret_cast<ib::sim::data::IDataPublisher*>(publisher);
    cppPublisher->Publish(std::vector<uint8_t>(&(data->pointer[0]), &(data->pointer[0]) + data->size));
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

