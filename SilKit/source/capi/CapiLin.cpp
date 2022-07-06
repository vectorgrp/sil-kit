// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "CapiImpl.hpp"
#include <cstring>

static void assign(SilKit::Services::Lin::LinFrame& cppFrame, const SilKit_LinFrame* cFrame) 
{
    cppFrame.id = static_cast<SilKit::Services::Lin::LinIdT>(cFrame->id);
    cppFrame.checksumModel = static_cast<SilKit::Services::Lin::LinChecksumModel>(cFrame->checksumModel);
    cppFrame.dataLength = static_cast<SilKit::Services::Lin::LinDataLengthT>(cFrame->dataLength);
    memcpy(cppFrame.data.data(), cFrame->data, 8);
}

static void assign(SilKit::Services::Lin::LinFrameResponse& cppFrameResponse, const SilKit_LinFrameResponse* cFrameResponse)
{
    assign(cppFrameResponse.frame, cFrameResponse->frame);
    cppFrameResponse.responseMode = static_cast<SilKit::Services::Lin::LinFrameResponseMode>(cFrameResponse->responseMode);
}

static void assign(std::vector<SilKit::Services::Lin::LinFrameResponse>& cppFrameResponses,
                   const SilKit_LinFrameResponse* cFrameResponses, uint32_t numFrameResponses)
{
    for (uint32_t i = 0; i < numFrameResponses; i++)
    {
        SilKit::Services::Lin::LinFrameResponse frameResponse;
        assign(frameResponse, &cFrameResponses[i]);
        cppFrameResponses.push_back(std::move(frameResponse));
    }
}

static void assign(SilKit::Services::Lin::LinControllerConfig& cppConfig, const SilKit_LinControllerConfig* cConfig)
{
    cppConfig.baudRate = static_cast<SilKit::Services::Lin::LinBaudRateT>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<SilKit::Services::Lin::LinControllerMode>(cConfig->controllerMode);
    assign(cppConfig.frameResponses, cConfig->frameResponses, cConfig->numFrameResponses);
}


extern "C" {

SilKit_ReturnCode SilKit_LinController_Create(SilKit_LinController** outLinController, SilKit_Participant* participant, const char* name, const char* network)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outLinController);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto cppLinController = cppParticipant->CreateLinController(name, network);
        if (cppLinController == nullptr)
        {
            return SilKit_ReturnCode_UNSPECIFIEDERROR;
        }
        *outLinController = reinterpret_cast<SilKit_LinController*>(cppLinController);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_Init(SilKit_LinController* controller, const SilKit_LinControllerConfig* config) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        auto cppControllerConfig = SilKit::Services::Lin::LinControllerConfig{};
        assign(cppControllerConfig, config);
        linController->Init(cppControllerConfig);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_Status(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_OUT_PARAMETER(outStatus);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        *outStatus = static_cast<SilKit_LinControllerStatus>(linController->Status());
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_SendFrame(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                         SilKit_LinFrameResponseType responseType)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        SilKit::Services::Lin::LinFrame cppFrame;
        assign(cppFrame, frame);
        linController->SendFrame(cppFrame, static_cast<SilKit::Services::Lin::LinFrameResponseType>(responseType));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_SendFrameHeader(SilKit_LinController* controller, SilKit_LinId linId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        linController->SendFrameHeader(static_cast<SilKit::Services::Lin::LinIdT>(linId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_SetFrameResponse(SilKit_LinController* controller,
                                                 const SilKit_LinFrameResponse* frameResponse)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frameResponse);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        SilKit::Services::Lin::LinFrame cppFrame;
        assign(cppFrame, frameResponse->frame);
        linController->SetFrameResponse(cppFrame,
                                        static_cast<SilKit::Services::Lin::LinFrameResponseMode>(frameResponse->responseMode));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_SetFrameResponses(SilKit_LinController* controller,
                                                  const SilKit_LinFrameResponse* frameResponses,
                                                  uint32_t numFrameResponses)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frameResponses);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        std::vector<SilKit::Services::Lin::LinFrameResponse> cppFrameResponses;
        assign(cppFrameResponses, frameResponses, numFrameResponses);
        linController->SetFrameResponses(std::move(cppFrameResponses));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_GoToSleep(SilKit_LinController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        linController->GoToSleep();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_GoToSleepInternal(SilKit_LinController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        linController->GoToSleepInternal();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_Wakeup(SilKit_LinController* controller) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        linController->Wakeup();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_WakeupInternal(SilKit_LinController* controller) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        linController->WakeupInternal();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_AddFrameStatusHandler(SilKit_LinController* controller, void* context,
                                                      SilKit_LinFrameStatusHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
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
                cFrameStatusEvent.interfaceId = SilKit_InterfaceIdentifier_LinFrameStatusEvent;
                cFrameStatusEvent.timestamp = (SilKit_NanosecondsTime)cppFrameStatusEvent.timestamp.count();
                cFrameStatusEvent.frame = &cFrame;
                cFrameStatusEvent.status = (SilKit_LinFrameStatus)cppFrameStatusEvent.status;

                handler(context, controller, &cFrameStatusEvent);
            });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_RemoveFrameStatusHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        cppController->RemoveFrameStatusHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_AddGoToSleepHandler(SilKit_LinController* controller, void* context,
                                                    SilKit_LinGoToSleepHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        *outHandlerId = (SilKit_HandlerId)linController->AddGoToSleepHandler(
            [handler, context, controller](SilKit::Services::Lin::ILinController* /*ctrl*/,
                                           const SilKit::Services::Lin::LinGoToSleepEvent& cppGoToSleepEvent) {
                SilKit_LinGoToSleepEvent goToSleepEvent{SilKit_InterfaceIdentifier_LinGoToSleepEvent,
                                                     (SilKit_NanosecondsTime)cppGoToSleepEvent.timestamp.count()};
                handler(context, controller, &goToSleepEvent);
            });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_RemoveGoToSleepHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        cppController->RemoveGoToSleepHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_AddWakeupHandler(SilKit_LinController* controller, void* context,
                                                 SilKit_LinWakeupHandler_t handler, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        *outHandlerId = (SilKit_HandlerId)linController->AddWakeupHandler(
            [handler, context, controller](SilKit::Services::Lin::ILinController* /*ctrl*/,
                                           const SilKit::Services::Lin::LinWakeupEvent& cppWakeupEvent) {
                SilKit_LinWakeupEvent wakeupEvent{SilKit_InterfaceIdentifier_LinWakeupEvent,
                                               (SilKit_NanosecondsTime)cppWakeupEvent.timestamp.count(),
                                               (SilKit_Direction)cppWakeupEvent.direction};
                handler(context, controller, &wakeupEvent);
        });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_LinController_RemoveWakeupHandler(SilKit_LinController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto cppController = reinterpret_cast<SilKit::Services::Lin::ILinController*>(controller);
        cppController->RemoveWakeupHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
