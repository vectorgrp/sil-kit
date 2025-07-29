// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

/*! \brief A pubsub/rpc node spec containing all matching relevant information */
typedef struct SilKit_DataSpec
{
    SilKit_StructHeader structHeader;
    const char* topic;
    const char* mediaType;
    SilKit_LabelList labelList;
} SilKit_DataSpec;

//! \brief An incoming DataMessage of a DataPublisher containing raw data and timestamp
typedef struct
{
    SilKit_StructHeader structHeader;
    //! Send timestamp of the event
    SilKit_NanosecondsTime timestamp;
    //! Data field containing the payload
    SilKit_ByteVector data;
} SilKit_DataMessageEvent;

/*! \brief Represents a handle to a data publisher instance */
typedef struct SilKit_DataPublisher SilKit_DataPublisher;
/*! \brief Represents a handle to a data subscriber instance */
typedef struct SilKit_DataSubscriber SilKit_DataSubscriber;

/*! \brief Handler type for incoming data message events on DataSubscribers. 
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param dataMessageEvent Contains the raw data and send timestamp.
*/
typedef void(SilKitFPTR* SilKit_DataMessageHandler_t)(void* context, SilKit_DataSubscriber* subscriber,
                                                      const SilKit_DataMessageEvent* dataMessageEvent);

/*! \brief Create a DataPublisher on the provided simulation participant with the provided properties.
* \param outPublisher Pointer to which the resulting DataPublisher reference will be written.
* \param participant The simulation participant for which the DataPublisher should be created.
* \param controllerName The name of this controller (UTF-8).
* \param dataSpec The specification of topic, media type and labels.
* \param history A number indicating the number of historic values that should be replayed for a new DataSubscriber.
* Restricted to {0|1}.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher,
                                                                   SilKit_Participant* participant,
                                                                   const char* controllerName,
                                                                   SilKit_DataSpec* dataSpec, uint8_t history);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_DataPublisher_Create_t)(SilKit_DataPublisher** outPublisher,
                                                                     SilKit_Participant* participant,
                                                                     const char* controllerName,
                                                                     SilKit_DataSpec* dataSpec, uint8_t history);

/*! \brief Create a DataSubscriber on the provided simulation participant with the provided properties.
* \param outSubscriber Pointer to which the resulting DataSubscriber reference will be written.
* \param participant The simulation participant for which the DataSubscriber should be created.
* \param controllerName The name of this controller (UTF-8).
* \param dataSpec The specification of topic, media type and labels.
* \param dataHandlerContext A user provided context that is reobtained on data reception in the dataHandler.
* \param dataHandler The handler that is called on data reception. Can be overwritten by \ref SilKit_DataSubscriber_SetDataMessageHandler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber,
                                                                    SilKit_Participant* participant,
                                                                    const char* controllerName,
                                                                    SilKit_DataSpec* dataSpec, void* dataHandlerContext,
                                                                    SilKit_DataMessageHandler_t dataHandler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_DataSubscriber_Create_t)(
    SilKit_DataSubscriber** outSubscriber, SilKit_Participant* participant, const char* controllerName,
    SilKit_DataSpec* dataSpec, void* dataHandlerContext, SilKit_DataMessageHandler_t dataHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Publish(SilKit_DataPublisher* self,
                                                                    const SilKit_ByteVector* data);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_DataPublisher_Publish_t)(SilKit_DataPublisher* self,
                                                                      const SilKit_ByteVector* data);

/*! \brief Sets / overwrites the default handler to be called on data reception.
* \param self The DataSubscriber for which the handler should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A handler that is called on data reception.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_SetDataMessageHandler(
    SilKit_DataSubscriber* self, void* context, SilKit_DataMessageHandler_t dataHandler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_DataSubscriber_SetDataMessageHandler_t)(
    SilKit_DataSubscriber* self, void* context, SilKit_DataMessageHandler_t dataHandler);

SILKIT_END_DECLS

#pragma pack(pop)
