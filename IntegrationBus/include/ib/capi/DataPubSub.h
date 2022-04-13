/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

/*! \brief represents a handle to a data publisher instance */
typedef struct ib_Data_Publisher ib_Data_Publisher;
/*! \brief represents a handle to a data subscriber instance */
typedef struct ib_Data_Subscriber ib_Data_Subscriber;

/*! \brief Handler type for incoming data on DataSubscribers.
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param data The data blob, containing the data of the message itself.
*/
typedef void (*ib_Data_Handler_t)(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data);

/*! \brief Handler type for new data sources.
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param mediaType The meta description of the data used by the new source.
* \param labels The labels used by the new source.
*/
typedef void (*ib_Data_NewDataSourceHandler_t)(void* context, ib_Data_Subscriber* subscriber, const char* topic,
                                               const char* mediaType, const ib_KeyValueList* labels);

/*! \brief Create a DataPublisher on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataPublisher reference will be written.
* \param participant The simulation participant for which the DataPublisher should be created.
* \param topic The topic string on which data should be published through this DataPublisher.
* \param mediaType A meta description of the data that will be published through this DataPublisher.
* \param labels A list of key-value pairs of this DataPublisher. The labels are received in the
* ib_Data_NewDataSourceHandler of a subscriber and are relevant for matching DataPublishers and DataSubscribers.
* \param history A number indicating the number of historic values that should be replayed for a new DataSubscriber.
* Restricted to {0|1}.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(ib_Data_Publisher** outPublisher,
                                                        ib_Participant* participant, const char* topic,
                                                        const char* mediaType, const ib_KeyValueList* labels,
                                                        uint8_t history);

typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(ib_Data_Publisher** outPublisher,
                                                   ib_Participant* participant, const char* topic,
                                                    const char* mediaType, const ib_KeyValueList* labels,
                                                    uint8_t history);

/*! \brief Create a DataSubscriber on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataSubscriber reference will be written.
* \param participant The simulation participant for which the DataSubscriber should be created.
* \param topic The topic string on which data should be published through this DataSubscriber.
* \param mediaType A meta description of the data that will be published through this DataSubscriber.
* \param labels A list of key-value pairs used to annotate which publications this subscriber is interested in.
* \param defaultDataHandlerContext A user provided context that is reobtained on data reception in the dataHandler.
* \param defaultDataHandler The default handler that is called on data reception as long as no specific handler on
* certain labels is registered. Can be overwritten by \ref ib_Data_Subscriber_SetDefaultReceiveDataHandler.
* \param newDataSourceContext A user provided context that is reobtained on invocation of the newDataSourceHandler.
* \param newDataSourceHandler A handler that is called if a new matching publisher is discovered.
*/
IntegrationBusAPI ib_ReturnCode
ib_Data_Subscriber_Create(ib_Data_Subscriber** outSubscriber, ib_Participant* participant, const char* topic,
                          const char* mediaType, const ib_KeyValueList* labels,
                          void* defaultDataHandlerContext, ib_Data_Handler_t defaultDataHandler,
                          void* newDataSourceContext, ib_Data_NewDataSourceHandler_t newDataSourceHandler);

typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(ib_Data_Subscriber** outSubscriber,
                                                     ib_Participant* participant, const char* topic,
                                                     const char* mediaType, const ib_KeyValueList* labels,
                                                     void* defaultDataHandlerContext,
                                                     ib_Data_Handler_t defaultDataHandler, void* newDataSourceContext,
                                                     ib_Data_NewDataSourceHandler_t newDataSourceHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Publish(ib_Data_Publisher* self, const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_Data_Publisher_Publish_t)(ib_Data_Publisher* self, const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_Data_Subscriber_SetDefaultReceiveDataHandler_t)(ib_Data_Subscriber* self, void* context,
                                                                          ib_Data_Handler_t dataHandler);

/*! \brief Sets / overwrites the default handler to be called on data reception.
* \param self The DataSubscriber for which the handler should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A handler that is called on data reception.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_SetDefaultReceiveDataHandler(ib_Data_Subscriber* self, void* context,
                                                                                ib_Data_Handler_t dataHandler);

/*! \brief Register a reception handler specific for given annotation details.
* Overwrites previously registered specific handlers if matching labels and dataExchangeFormat is used.
* If a specific handler is available, the default handler will not be called on data reception.
* \param self The DataSubscriber for which the specific handler should be set.
* \param mediaType The meta description of the data that has to match the mediaType of the DataPublisher to use the
* specific handler.
* \param labels The labels that have to match the labels provided by the DataPublisher to use the specific handler.
* \param context A user provided context that is reobtained on data reception in the specific handler.
* \param dataHandler A handler that is called on data reception by publishers with matching annotations.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_RegisterSpecificDataHandler(
    ib_Data_Subscriber* self, const char* mediaType, const ib_KeyValueList* labels, void* context,
    ib_Data_Handler_t dataHandler);

typedef ib_ReturnCode (*ib_Data_Subscriber_RegisterSpecificDataHandler_t)(ib_Data_Subscriber* self,
                                                                          const char* mediaType,
                                                                          const ib_KeyValueList* labels, void* context,
                                                                          ib_Data_Handler_t dataHandler);

IB_END_DECLS

#pragma pack(pop)
