/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


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

/*! \brief represents a handle to a data publisher instance */
typedef struct SilKit_DataPublisher SilKit_DataPublisher;
/*! \brief represents a handle to a data subscriber instance */
typedef struct SilKit_DataSubscriber SilKit_DataSubscriber;

/*! \brief Handler type for incoming data message events on DataSubscribers. 
* \param context The context that the user provided on registration.
* \param subscriber The affected subscriber.
* \param dataMessageEvent Contains the raw data and send timestamp.
*/
typedef void (SilKitFPTR *SilKit_DataMessageHandler_t)(void* context, SilKit_DataSubscriber* subscriber,
    const SilKit_DataMessageEvent* dataMessageEvent);

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
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Create(SilKit_DataPublisher** outPublisher,
                                                        SilKit_Participant* participant, const char* controllerName,
                                                        SilKit_DataSpec* dataSpec, uint8_t history);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_DataPublisher_Create_t)(SilKit_DataPublisher** outPublisher,
                                                           SilKit_Participant* participant, const char* controllerName,
                                                           SilKit_DataSpec* dataSpec, uint8_t history);

/*! \brief Create a DataSubscriber on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting DataSubscriber reference will be written.
* \param participant The simulation participant for which the DataSubscriber should be created.
* \param controllerName The name of this controller.
* \param topic The topic string on which data should be published through this DataSubscriber.
* \param mediaType A meta description of the data that will be published through this DataSubscriber.
* \param labels A list of key-value pairs used to annotate which publications this subscriber is interested in.
* \param defaultDataHandlerContext A user provided context that is reobtained on data reception in the dataHandler.
* \param defaultDataHandler The default handler that is called on data reception as long as no explicit handler on 
* certain labels is registered. Can be overwritten by \ref SilKit_DataSubscriber_SetDataMessageHandler.
* \param newDataSourceContext A user provided context that is reobtained on invocation of the newDataSourceHandler.
* \param newDataSourceHandler A handler that is called if a new matching publisher is discovered.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_Create(SilKit_DataSubscriber** outSubscriber,
                                                         SilKit_Participant* participant, const char* controllerName,
                                                         SilKit_DataSpec* dataSpec,
                                                         void* defaultDataHandlerContext,
                                                         SilKit_DataMessageHandler_t defaultDataHandler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_DataSubscriber_Create_t)(SilKit_DataSubscriber** outSubscriber,
                                                            SilKit_Participant* participant, const char* controllerName,
                                                            SilKit_DataSpec* dataSpec,
                                                            void* defaultDataHandlerContext,
                                                            SilKit_DataMessageHandler_t defaultDataHandler);

/*! \brief Publish data through the provided DataPublisher
* \param self The DataPublisher that should publish the data.
* \param data The data that should be published.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataPublisher_Publish(SilKit_DataPublisher* self, const SilKit_ByteVector* data);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_DataPublisher_Publish_t)(SilKit_DataPublisher* self, const SilKit_ByteVector* data);

/*! \brief Sets / overwrites the default handler to be called on data reception.
* \param self The DataSubscriber for which the handler should be set.
* \param context A user provided context, that is reobtained on data reception in the dataHandler.
* \param dataHandler A handler that is called on data reception.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_DataSubscriber_SetDataMessageHandler(
    SilKit_DataSubscriber* self, void* context, SilKit_DataMessageHandler_t dataHandler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_DataSubscriber_SetDataMessageHandler_t)(SilKit_DataSubscriber* self, void* context,
                                                                           SilKit_DataMessageHandler_t dataHandler);

SILKIT_END_DECLS

#pragma pack(pop)
