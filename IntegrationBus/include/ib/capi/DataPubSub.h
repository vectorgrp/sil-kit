/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//! \brief An incoming DataMessage of a DataPublisher containing raw data and timestamp
typedef struct
{
    SilKit_InterfaceIdentifier interfaceId;
    //! Send timestamp of the event
    SilKit_NanosecondsTime timestamp;
    //! Data field containing the payload
    SilKit_ByteVector data; 
} SilKit_DataMessageEvent;

//! \brief Information about a newly discovered DataPublisher
typedef struct
{
    SilKit_InterfaceIdentifier interfaceId;
    //! Reception timestamp of the event
    SilKit_NanosecondsTime timestamp;
    //! The topic string of the discovered DataPublisher.
    const char* topic;
    //! The mediaType of the discovered DataPublisher.
    const char* mediaType;
    //! The labels of the discovered DataPublisher.
    SilKit_KeyValueList* labels;
} SilKit_NewDataPublisherEvent;

/*! \brief represents a handle to a data publisher instance */
typedef struct SilKit_DataPublisher SilKit_DataPublisher;
/*! \brief represents a handle to a data subscriber instance */
typedef struct SilKit_DataSubscriber SilKit_DataSubscriber;

/*! \brief Handler type for incoming data message events on DataSubscribers. 
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param dataMessageEvent Contains the raw data and send timestamp.
*/
typedef void (*SilKit_DataMessageHandler_t)(void* context, SilKit_DataSubscriber* subscriber, 
    const SilKit_DataMessageEvent* dataMessageEvent);

/*! \brief Handler type for new data publishers.
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param newDataPublisherEvent Contains information about the new DataPublisher and the reception timestamp.
*/
typedef void (*SilKit_NewDataPublisherHandler_t)(void* context, SilKit_DataSubscriber* subscriber,
                                                  const SilKit_NewDataPublisherEvent* newDataPublisherEvent);

/*! \brief Create a DataPublisher on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataPublisher reference will be written.
* \param participant The simulation participant for which the DataPublisher should be created.
* \param controllerName The name of this controller.
* \param topic The topic string on which data should be published through this DataPublisher.
* \param mediaType A meta description of the data that will be published through this DataPublisher.
* \param labels A list of key-value pairs of this DataPublisher. The labels are received in the
* SilKit_NewDataSourceHandler of a subscriber and are relevant for matching DataPublishers and DataSubscribers.
* \param history A number indicating the number of historic values that should be replayed for a new DataSubscriber.
* Restricted to {0|1}.
*/
SilKitAPI SilKit_ReturnCode SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher, SilKit_Participant* participant,
                                                         const char* controllerName, const char* topic,
                                                         const char* mediaType, const SilKit_KeyValueList* labels,
                                                         uint8_t history);

typedef SilKit_ReturnCode (*SilKit_DataPublisher_Create_t)(SilKit_DataPublisher** outPublisher, SilKit_Participant* participant,
                                                    const char* controllerName, const char* topic,
                                                    const char* mediaType, const SilKit_KeyValueList* labels,
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
* certain labels is registered. Can be overwritten by \ref SilKit_DataSubscriber_SetDefaultDataMessageHandler.
* \param newDataSourceContext A user provided context that is reobtained on invocation of the newDataSourceHandler.
* \param newDataSourceHandler A handler that is called if a new matching publisher is discovered.
*/
SilKitAPI SilKit_ReturnCode
SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber, SilKit_Participant* participant, const char* controllerName,
                          const char* topic, const char* mediaType, const SilKit_KeyValueList* labels,
                          void* defaultDataHandlerContext, SilKit_DataMessageHandler_t defaultDataHandler,
                          void* newDataSourceContext, SilKit_NewDataPublisherHandler_t newDataSourceHandler);

typedef SilKit_ReturnCode (*SilKit_DataSubscriber_Create_t)(SilKit_DataSubscriber** outSubscriber, SilKit_Participant* participant,
                                                     const char* controllerName, const char* topic,
                                                     const char* mediaType, const SilKit_KeyValueList* labels,
                                                     void* defaultDataHandlerContext,
                                                     SilKit_DataMessageHandler_t defaultDataHandler,
                                                     void* newDataSourceContext,
                                                     SilKit_NewDataPublisherHandler_t newDataSourceHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
SilKitAPI SilKit_ReturnCode SilKit_DataPublisher_Publish(SilKit_DataPublisher* self, const SilKit_ByteVector* data);

typedef SilKit_ReturnCode (*SilKit_DataPublisher_Publish_t)(SilKit_DataPublisher* self, const SilKit_ByteVector* data);

/*! \brief Sets / overwrites the default handler to be called on data reception.
* \param self The DataSubscriber for which the handler should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A handler that is called on data reception.
*/
SilKitAPI SilKit_ReturnCode SilKit_DataSubscriber_SetDefaultDataMessageHandler(
    SilKit_DataSubscriber* self, void* context, SilKit_DataMessageHandler_t dataHandler);

typedef SilKit_ReturnCode (*SilKit_DataSubscriber_SetDefaultDataMessageHandler_t)(SilKit_DataSubscriber* self, void* context,
                                                                           SilKit_DataMessageHandler_t dataHandler);

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
SilKitAPI SilKit_ReturnCode SilKit_DataSubscriber_AddExplicitDataMessageHandler(
    SilKit_DataSubscriber* self, void* context, SilKit_DataMessageHandler_t dataHandler, const char* mediaType,
    const SilKit_KeyValueList* labels, SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (*SilKit_DataSubscriber_AddExplicitDataMessageHandler_t)(SilKit_DataSubscriber* self, void* context,
                                                                            SilKit_DataMessageHandler_t dataHandler,
                                                                            const char* mediaType,
                                                                            const SilKit_KeyValueList* labels,
                                                                            SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_DataMessageHandler_t by SilKit_HandlerId on this subscriber
*
* \param self The subscriber for which the explicit handler should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKit_DataSubscriber_RemoveExplicitDataMessageHandler(SilKit_DataSubscriber* self,
                                                                                    SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_DataSubscriber_RemoveExplicitDataMessageHandler_t)(SilKit_DataSubscriber* self,
                                                                               SilKit_HandlerId handlerId);

SILKIT_END_DECLS

#pragma pack(pop)
