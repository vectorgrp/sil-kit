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

#include "services/lin/LinControllerExtensionsImpl.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

#include "CapiImpl.hpp"

#include <cstring>


namespace {

void assign(SilKit::Services::Lin::LinFrame& cppFrame, const SilKit_LinFrame* cFrame)
{
    cppFrame.id = static_cast<SilKit::Services::Lin::LinId>(cFrame->id);
    cppFrame.checksumModel = static_cast<SilKit::Services::Lin::LinChecksumModel>(cFrame->checksumModel);
    cppFrame.dataLength = static_cast<SilKit::Services::Lin::LinDataLength>(cFrame->dataLength);
    memcpy(cppFrame.data.data(), cFrame->data, 8);
}

void assign(SilKit::Services::Lin::LinFrameResponse& cppFrameResponse, const SilKit_LinFrameResponse* cFrameResponse)
{
    assign(cppFrameResponse.frame, cFrameResponse->frame);
    cppFrameResponse.responseMode =
        static_cast<SilKit::Services::Lin::LinFrameResponseMode>(cFrameResponse->responseMode);
}

void assign(std::vector<SilKit::Services::Lin::LinFrameResponse>& cppFrameResponses,
            const SilKit_LinFrameResponse* cFrameResponses, size_t numFrameResponses)
{
    for (size_t i = 0; i < numFrameResponses; i++)
    {
        SilKit::Services::Lin::LinFrameResponse frameResponse;
        assign(frameResponse, &cFrameResponses[i]);
        cppFrameResponses.push_back(std::move(frameResponse));
    }
}

void assign(SilKit::Services::Lin::LinControllerConfig& cppConfig, const SilKit_LinControllerConfig* cConfig)
{
    cppConfig.baudRate = static_cast<SilKit::Services::Lin::LinBaudRate>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<SilKit::Services::Lin::LinControllerMode>(cConfig->controllerMode);
    assign(cppConfig.frameResponses, cConfig->frameResponses, cConfig->numFrameResponses);
}

void assign(SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& cppConfig, const SilKit_Experimental_LinControllerDynamicConfig* cConfig)
{
    cppConfig.baudRate = static_cast<SilKit::Services::Lin::LinBaudRate>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<SilKit::Services::Lin::LinControllerMode>(cConfig->controllerMode);
}

// Assign the cppLinSlaveConfiguration to cLinSlaveConfiguration
void assign(SilKit_Experimental_LinSlaveConfiguration** cLinSlaveConfiguration,
            const SilKit::Experimental::Services::Lin::LinSlaveConfiguration& cppLinSlaveConfiguration)
{
    memset((*cLinSlaveConfiguration)->isLinIdResponding, SilKit_False, 64);
    for (auto&& linId : cppLinSlaveConfiguration.respondingLinIds)
    {
        (*cLinSlaveConfiguration)->isLinIdResponding[linId] = SilKit_True;
    }
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_LinController_Create(SilKit_LinController** outLinController, SilKit_Participant* participant, const char* name, const char* network)
try
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outLinController);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto cppLinController = cppParticipant->CreateLinController(name, network);
    if (cppLinController == nullptr)
    {
        return SilKit_ReturnCode_UNSPECIFIEDERROR;
    }
    *outLinController = reinterpret_cast<SilKit_LinController*>(cppLinController);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_Init(SilKit_LinController* controller, const SilKit_LinControllerConfig* config)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    ASSERT_VALID_STRUCT_HEADER(config);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    auto cppControllerConfig = SilKit::Services::Lin::LinControllerConfig{};
    assign(cppControllerConfig, config);
    linController->Init(cppControllerConfig);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_SetFrameResponse(SilKit_LinController* controller,
                                                                   const SilKit_LinFrameResponse* response)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(response);
    ASSERT_VALID_STRUCT_HEADER(response);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    auto cppFrameResponse = SilKit::Services::Lin::LinFrameResponse{};
    assign(cppFrameResponse.frame, response->frame);
    cppFrameResponse.responseMode = static_cast<SilKit::Services::Lin::LinFrameResponseMode>(response->responseMode);
    linController->SetFrameResponse(cppFrameResponse);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_Status(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_OUT_PARAMETER(outStatus);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    *outStatus = static_cast<SilKit_LinControllerStatus>(linController->Status());
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrame(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                         SilKit_LinFrameResponseType responseType)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    ASSERT_VALID_STRUCT_HEADER(frame);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    SilKit::Services::Lin::LinFrame cppFrame;
    assign(cppFrame, frame);
    linController->SendFrame(cppFrame, static_cast<SilKit::Services::Lin::LinFrameResponseType>(responseType));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrameHeader(SilKit_LinController* controller, SilKit_LinId linId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    linController->SendFrameHeader(static_cast<SilKit::Services::Lin::LinId>(linId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_UpdateTxBuffer(SilKit_LinController* controller,
                                                 const SilKit_LinFrame* frame)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    ASSERT_VALID_STRUCT_HEADER(frame);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    SilKit::Services::Lin::LinFrame cppFrame;
    assign(cppFrame, frame);
    linController->UpdateTxBuffer(cppFrame);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleep(SilKit_LinController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    linController->GoToSleep();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleepInternal(SilKit_LinController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    linController->GoToSleepInternal();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_Wakeup(SilKit_LinController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    linController->Wakeup();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_WakeupInternal(SilKit_LinController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    linController->WakeupInternal();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_GetSlaveConfiguration(SilKit_LinController* controller,
                                                             SilKit_Experimental_LinSlaveConfiguration* outLinSlaveConfiguration)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_OUT_PARAMETER(outLinSlaveConfiguration);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    auto cppSlaveConfiguration = SilKit::Experimental::Services::Lin::GetSlaveConfigurationImpl(linController);
    SilKit_Struct_Init(SilKit_Experimental_LinSlaveConfiguration, *outLinSlaveConfiguration);
    assign(&outLinSlaveConfiguration, cppSlaveConfiguration);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_AddFrameStatusHandler(SilKit_LinController* controller, void* context,
                                                      SilKit_LinFrameStatusHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    *outHandlerId = (SilKit_HandlerId)linController->AddFrameStatusHandler(
        [handler, context, controller](
            SilKit::Services::Lin::ILinController* /*ctrl*/, const SilKit::Services::Lin::LinFrameStatusEvent& cppFrameStatusEvent)
        {
            SilKit_LinFrame cFrame;
            cFrame.id = static_cast<SilKit_LinId>(cppFrameStatusEvent.frame.id);
            cFrame.checksumModel = static_cast<SilKit_LinChecksumModel>(cppFrameStatusEvent.frame.checksumModel);
            cFrame.dataLength = static_cast<SilKit_LinDataLength>(cppFrameStatusEvent.frame.dataLength);
            memcpy(cFrame.data, cppFrameStatusEvent.frame.data.data(), 8);

            SilKit_LinFrameStatusEvent cFrameStatusEvent{};
            SilKit_Struct_Init(SilKit_LinFrameStatusEvent, cFrameStatusEvent);
            cFrameStatusEvent.timestamp = (SilKit_NanosecondsTime)cppFrameStatusEvent.timestamp.count();
            cFrameStatusEvent.frame = &cFrame;
            cFrameStatusEvent.status = (SilKit_LinFrameStatus)cppFrameStatusEvent.status;

            handler(context, controller, &cFrameStatusEvent);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveFrameStatusHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    cppController->RemoveFrameStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_AddGoToSleepHandler(SilKit_LinController* controller, void* context,
                                                    SilKit_LinGoToSleepHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    *outHandlerId = (SilKit_HandlerId)linController->AddGoToSleepHandler(
        [handler, context, controller](SilKit::Services::Lin::ILinController* /*ctrl*/,
                                       const SilKit::Services::Lin::LinGoToSleepEvent& cppGoToSleepEvent) {
            SilKit_LinGoToSleepEvent goToSleepEvent{};
            SilKit_Struct_Init(SilKit_LinGoToSleepEvent, goToSleepEvent);
            goToSleepEvent.timestamp = (SilKit_NanosecondsTime)cppGoToSleepEvent.timestamp.count();
            handler(context, controller, &goToSleepEvent);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveGoToSleepHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    cppController->RemoveGoToSleepHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_AddWakeupHandler(SilKit_LinController* controller, void* context,
                                                 SilKit_LinWakeupHandler_t handler, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    *outHandlerId = (SilKit_HandlerId)linController->AddWakeupHandler(
        [handler, context, controller](SilKit::Services::Lin::ILinController* /*ctrl*/,
                                       const SilKit::Services::Lin::LinWakeupEvent& cppWakeupEvent) {
            SilKit_LinWakeupEvent wakeupEvent{};
            SilKit_Struct_Init(SilKit_LinWakeupEvent, wakeupEvent);
            wakeupEvent.timestamp = (SilKit_NanosecondsTime)cppWakeupEvent.timestamp.count();
            wakeupEvent.direction = (SilKit_Direction)cppWakeupEvent.direction;
            handler(context, controller, &wakeupEvent);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveWakeupHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    cppController->RemoveWakeupHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(
    SilKit_LinController* controller, void* context, SilKit_Experimental_LinSlaveConfigurationHandler_t handler,
    SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    *outHandlerId = (SilKit_HandlerId)SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandlerImpl(
        linController,
        [handler, context, controller](
            SilKit::Services::Lin::ILinController* /*ctrl*/,
            const SilKit::Experimental::Services::Lin::LinSlaveConfigurationEvent& cppLinSlaveConfigurationEvent) {
            SilKit_Experimental_LinSlaveConfigurationEvent slaveConfigurationEvent{};
            SilKit_Struct_Init(SilKit_Experimental_LinSlaveConfigurationEvent, slaveConfigurationEvent);
            slaveConfigurationEvent.timestamp =
                (SilKit_NanosecondsTime)cppLinSlaveConfigurationEvent.timestamp.count();
            handler(context, controller, &slaveConfigurationEvent);
        });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandlerImpl(
        cppController, static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_InitDynamic(
    SilKit_LinController* controller, const SilKit_Experimental_LinControllerDynamicConfig* config)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    ASSERT_VALID_STRUCT_HEADER(config);
    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    auto cppDynamicConfig = SilKit::Experimental::Services::Lin::LinControllerDynamicConfig{};
    assign(cppDynamicConfig, config);
    SilKit::Experimental::Services::Lin::InitDynamicImpl(linController, cppDynamicConfig);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddFrameHeaderHandler(
    SilKit_LinController* controller, void* context, SilKit_Experimental_LinFrameHeaderHandler_t handler,
    SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    auto cppHandlerId = SilKit::Experimental::Services::Lin::AddFrameHeaderHandlerImpl(
        linController, [handler, context, controller](auto* /*ctrl*/, auto&& cppEvent) {
            SilKit_Experimental_LinFrameHeaderEvent cEvent{};
            SilKit_Struct_Init(SilKit_LinWakeupEvent, cEvent);
            cEvent.timestamp = (SilKit_NanosecondsTime)cppEvent.timestamp.count();
            cEvent.id = (SilKit_LinId)cppEvent.id;
            handler(context, controller, &cEvent);
        });
    *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL
SilKit_Experimental_LinController_RemoveFrameHeaderHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    SilKit::Experimental::Services::Lin::RemoveFrameHeaderHandlerImpl(linController,
                                                                      static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_SendDynamicResponse(SilKit_LinController* controller,
    const SilKit_LinFrame* frame)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);

    auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
    SilKit::Services::Lin::LinFrame cppLinFrame;
    assign(cppLinFrame, frame);
    SilKit::Experimental::Services::Lin::SendDynamicResponseImpl(linController, cppLinFrame);

    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
