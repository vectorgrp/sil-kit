/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

//! \brief An incoming DataMessage of a DataPublisher containing raw data and timestamp
typedef struct
{
    ib_InterfaceIdentifier interfaceId;
    //! Send timestamp of the event
    ib_NanosecondsTime timestamp;
    //! Data field containing the payload
    ib_ByteVector data; 
} ib_Data_DataMessageEvent;

//! \brief Information about a newly discovered DataPublisher
typedef struct
{
    ib_InterfaceIdentifier interfaceId;
    //! Reception timestamp of the event
    ib_NanosecondsTime timestamp;
    //! The topic string of the discovered DataPublisher.
    const char* topic;
    //! The mediaType of the discovered DataPublisher.
    const char* mediaType;
    //! The labels of the discovered DataPublisher.
    ib_KeyValueList* labels;
} ib_Data_NewDataPublisherEvent;

/*! \brief represents a handle to a data publisher instance */
typedef struct ib_Data_Publisher ib_Data_Publisher;
/*! \brief represents a handle to a data subscriber instance */
typedef struct ib_Data_Subscriber ib_Data_Subscriber;

/*! \brief Handler type for incoming data message events on DataSubscribers. 
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param dataMessageEvent Contains the raw data and send timestamp.
*/
typedef void (*ib_Data_DataMessageHandler_t)(void* context, ib_Data_Subscriber* subscriber, 
    const ib_Data_DataMessageEvent* dataMessageEvent);

/*! \brief Handler type for new data publishers.
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param newDataPublisherEvent Contains information about the new DataPublisher and the reception timestamp.
*/
typedef void (*ib_Data_NewDataPublisherHandler_t)(void* context, ib_Data_Subscriber* subscriber,
                                                  const ib_Data_NewDataPublisherEvent* newDataPublisherEvent);

/*! \brief Create a DataPublisher on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataPublisher reference will be written.
* \param participant The simulation participant for which the DataPublisher should be created.
* \param controllerName The name of this controller.
* \param topic The topic string on which data should be published through this DataPublisher.
* \param mediaType A meta description of the data that will be published through this DataPublisher.
* \param labels A list of key-value pairs of this DataPublisher. The labels are received in the
* ib_Data_NewDataSourceHandler of a subscriber and are relevant for matching DataPublishers and DataSubscribers.
* \param history A number indicating the number of historic values that should be replayed for a new DataSubscriber.
* Restricted to {0|1}.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Create(ib_Data_Publisher** outPublisher, ib_Participant* participant,
                                                         const char* controllerName, const char* topic,
                                                         const char* mediaType, const ib_KeyValueList* labels,
                                                         uint8_t history);

typedef ib_ReturnCode (*ib_Data_Publisher_Create_t)(ib_Data_Publisher** outPublisher, ib_Participant* participant,
                                                    const char* controllerName, const char* topic,
                                                    const char* mediaType, const ib_KeyValueList* labels,
                                                    uint8_t history);

/*! \brief Create a DataSubscriber on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataSubscriber reference will be written.
* \param participant The simulation participant for which the DataSubscriber should be created.
* \param controllerName The name of this controller.
* \param topic The topic string on which data should be published through this DataSubscriber.
* \param mediaType A meta description of the data that will be published through this DataSubscriber.
* \param labels A list of key-value pairs used to annotate which publications this subscriber is interested in.
* \param defaultDataHandlerContext A user provided context that is reobtained on data reception in the dataHandler.
* \param defaultDataHandler The default handler that is called on data reception as long as no explicit handler on 
* certain labels is registered. Can be overwritten by \ref ib_Data_Subscriber_SetDefaultDataMessageHandler.
* \param newDataSourceContext A user provided context that is reobtained on invocation of the newDataSourceHandler.
* \param newDataSourceHandler A handler that is called if a new matching publisher is discovered.
*/
IntegrationBusAPI ib_ReturnCode
ib_Data_Subscriber_Create(ib_Data_Subscriber** outSubscriber, ib_Participant* participant, const char* controllerName,
                          const char* topic, const char* mediaType, const ib_KeyValueList* labels,
                          void* defaultDataHandlerContext, ib_Data_DataMessageHandler_t defaultDataHandler,
                          void* newDataSourceContext, ib_Data_NewDataPublisherHandler_t newDataSourceHandler);

typedef ib_ReturnCode (*ib_Data_Subscriber_Create_t)(ib_Data_Subscriber** outSubscriber, ib_Participant* participant,
                                                     const char* controllerName, const char* topic,
                                                     const char* mediaType, const ib_KeyValueList* labels,
                                                     void* defaultDataHandlerContext,
                                                     ib_Data_DataMessageHandler_t defaultDataHandler,
                                                     void* newDataSourceContext,
                                                     ib_Data_NewDataPublisherHandler_t newDataSourceHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Publisher_Publish(ib_Data_Publisher* self, const ib_ByteVector* data);

typedef ib_ReturnCode (*ib_Data_Publisher_Publish_t)(ib_Data_Publisher* self, const ib_ByteVector* data);

/*! \brief Sets / overwrites the default handler to be called on data reception.
* \param self The DataSubscriber for which the handler should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A handler that is called on data reception.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_SetDefaultDataMessageHandler(
    ib_Data_Subscriber* self, void* context, ib_Data_DataMessageHandler_t dataHandler);

typedef ib_ReturnCode (*ib_Data_Subscriber_SetDefaultDataMessageHandler_t)(ib_Data_Subscriber* self, void* context,
                                                                           ib_Data_DataMessageHandler_t dataHandler);

/*! \brief Register a reception handler explicit for given annotation details.
* Overwrites previously registered explicit handlers if the same labels and media type are used.
* If a explicit handler is available, the default handler will not be called on data reception.
* \param self The DataSubscriber for which the explicit handler should be set.
* \param mediaType The meta description of the data that has to match the mediaType of the DataPublisher to use the
* explicit handler.
* \param labels The labels that have to match the labels provided by the DataPublisher to use the explicit handler.
* \param context A user provided context that is reobtained on data reception in the explicit handler.
* \param dataHandler A handler that is called on data reception by publishers with matching annotations.
*/
IntegrationBusAPI ib_ReturnCode ib_Data_Subscriber_AddExplicitDataMessageHandler(
    ib_Data_Subscriber* self, void* context, ib_Data_DataMessageHandler_t dataHandler, const char* mediaType,
    const ib_KeyValueList* labels);

typedef ib_ReturnCode (*ib_Data_Subscriber_AddExplicitDataMessageHandler_t)(ib_Data_Subscriber* self, void* context,
                                                                            ib_Data_DataMessageHandler_t dataHandler,
                                                                            const char* mediaType,
                                                                            const ib_KeyValueList* labels);

IB_END_DECLS

#pragma pack(pop)
