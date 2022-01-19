/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

/*! \brief The ib_DataExchangeFormat provides meta information to data of the data exchange service.
*   Except for this descriptional pupose, it is used for matching DataProviders and DataSubscribers.
*/
typedef struct {
    ib_InterfaceIdentifier interfaceId;
    /*! \brief The media type of the data as specified by RFC2046 
    * (e.g. "application/xml", "application/vnd.google.protobuf", ...)
    * A null pointer is considered an invalid value. Only valid RFC2046 values are considered valid.
    * The only exception to this rule is an empty string. 
    * An empty mediaType string is interpreted as undefined and matches any mediaType
    */
    const char* mediaType; 
} ib_DataExchangeFormat;

/*! \brief represents a handle to a data publisher instance */
typedef struct ib_DataPublisher ib_DataPublisher;
/*! \brief represents a handle to a data subscriber instance */
typedef struct ib_DataSubscriber ib_DataSubscriber;

/*! \brief Callback type for incoming data on DataSubscribers. 
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param data The data blob, containing the data of the message itself.
* \param dataExchangeFormat A meta description of the received data.
*/
typedef void (*ib_DataHandler_t)(void* context, ib_DataSubscriber* subscriber, 
    const ib_ByteVector* data, const ib_DataExchangeFormat* dataExchangeFormat);

/*! \brief Create a DataPublisher on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataPublisher reference will be written.
* \param participant The simulation participant for which the DataPublisher should be created.
* \param topic The topic string on which data should be published through this DataPublisher.
* \param dataExchangeFormat A meta description of the data that will be published through this DataPublisher.
* \param history A number indicating the number of historic values that should be replayed for a new DataSubscriber.
* Restricted to {0|1}.
*/
IntegrationBusAPI ib_ReturnCode ib_DataPublisher_Create(
  ib_DataPublisher** outPublisher, 
  ib_SimulationParticipant* participant,
  const char* topic,
  ib_DataExchangeFormat* dataExchangeFormat,
  uint8_t history);

typedef ib_ReturnCode(*ib_DataPublisher_Create_t)(
  ib_DataPublisher** outPublisher, 
  ib_SimulationParticipant* participant,
  const char* topic,
  ib_DataExchangeFormat* dataExchangeFormat,
  uint8_t history);

/*! \brief Create a DataSubscriber on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataSubscriber reference will be written.
* \param participant The simulation participant for which the DataSubscriber should be created.
* \param topic The topic string on which data should be published through this DataSubscriber.
* \param dataExchangeFormat A meta description of the data that will be published through this DataSubscriber.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A callback that is called on data reception. Has to be specified directly to be able to handle
* historic data received on DataSubscriber creation. Can be overwritten by \ref ib_DataSubscriber_SetReceiveDataHandler.
* 
* If available, historic values are rolled out directly on creation of the DataSubscriber and the dataHandler is called
* before ib_DataSubscriber_Create returns.
*/
IntegrationBusAPI ib_ReturnCode ib_DataSubscriber_Create(
  ib_DataSubscriber** outSubscriber, 
  ib_SimulationParticipant* participant,
  const char* topic,
  ib_DataExchangeFormat* dataExchangeFormat,
  void* context,
  ib_DataHandler_t dataHandler);

typedef ib_ReturnCode(*ib_DataSubscriber_Create_t)(
  ib_DataSubscriber** outSubscriber,
  ib_SimulationParticipant* participant, 
  const char* topic,
  ib_DataExchangeFormat* dataExchangeFormat,
  void* context,
  ib_DataHandler_t dataHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
IntegrationBusAPI ib_ReturnCode ib_DataPublisher_Publish(ib_DataPublisher* self, const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_DataPublisher_Publish_t)(ib_DataPublisher* self, const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_DataSubscriber_SetReceiveDataHandler_t)(ib_DataSubscriber* self, void* context, 
    ib_DataHandler_t dataHandler);
/*! \brief Sets the callback handler to be called on data reception.
* Overwrites potentially earlier set callback handlers.
* \param self The DataSubscriber for which the callback should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A callback that is called on data reception.
*/
IntegrationBusAPI ib_ReturnCode ib_DataSubscriber_SetReceiveDataHandler(ib_DataSubscriber* self, void* context, 
    ib_DataHandler_t dataHandler);

IB_END_DECLS

#pragma pack(pop)
