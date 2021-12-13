#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/generic/all.hpp"
#include "ib/sim/generic/string_utils.hpp"

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
  //ASSERT_VALID_POINTER_PARAMETER(dataTypeInfo); // may this be null ?
  CAPI_ENTER
  {
    if (dataTypeInfo != NULL && (std::strcmp(dataTypeInfo->mediaType, "")))
    {
      ib_error_string = "This integration bus implementation currently does not support dataTypeInfos that are more specific than wildcards.";
      return ib_ReturnCode_NOTSUPPORTED;
    }
    if (history != 0)
    {
      ib_error_string = "This integration bus implementation currently only supports a history length of 0.";
      return ib_ReturnCode_NOTSUPPORTED;
    }

    std::string strTopic(topic);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto genericPublisher = comAdapter->CreateGenericPublisher(strTopic);
    *out = reinterpret_cast<ib_DataPublisher*>(genericPublisher);
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
  //ASSERT_VALID_HANDLER_PARAMETER(dataHandler);
  //ASSERT_VALID_POINTER_PARAMETER(dataTypeInfo); // may this be null ?
  CAPI_ENTER
  {
    if (dataTypeInfo != NULL && (std::strcmp(dataTypeInfo->mediaType,"") != 0))
    {
      ib_error_string = "This integration bus implementation currently does not support data exchange formats that are more specific than wildcards.";
      return ib_ReturnCode_NOTSUPPORTED;
    }

    std::string strTopic(topic);
    auto comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
    auto genericSubscriber = comAdapter->CreateGenericSubscriber(strTopic);
    *out = reinterpret_cast<ib_DataSubscriber*>(genericSubscriber);

    // Register Data Handler if provided
    if (dataHandler != NULL)
    {
      ib_DataSubscriber_SetReceiveDataHandler(*out, context, dataHandler);
    }

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
    auto cppSubscriber = reinterpret_cast<ib::sim::generic::IGenericSubscriber*>(subscriber);
    cppSubscriber->SetReceiveMessageHandler(
      [dataHandler, context, subscriber](ib::sim::generic::IGenericSubscriber* cppSubscriber, const std::vector<uint8_t>& data)
      {
        uint8_t* payloadPointer = NULL;
        if (data.size() > 0)
        {
          payloadPointer = (uint8_t* const)&(data[0]);
        }
        const ib_ByteVector ccdata{ payloadPointer, data.size() };
        dataHandler(context, subscriber,  &ccdata);
      });
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

ib_ReturnCode ib_DataPublisher_Publish(ib_DataPublisher* publisher, const ib_ByteVector* data)
{
  ASSERT_VALID_POINTER_PARAMETER(publisher);
  ASSERT_VALID_HANDLER_PARAMETER(data);
  CAPI_ENTER
  {
    auto cppPublisher = reinterpret_cast<ib::sim::generic::IGenericPublisher*>(publisher);
    cppPublisher->Publish(std::vector<uint8_t>(&(data->pointer[0]), &(data->pointer[0]) + data->size));
    return ib_ReturnCode_SUCCESS;
  }
  CAPI_LEAVE
}

