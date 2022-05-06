/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "CapiImpl.h"
#include <cstring>

static void assign(ib::sim::lin::LinFrame& cppFrame, const ib_Lin_Frame* cFrame) 
{
    cppFrame.id = static_cast<ib::sim::lin::LinIdT>(cFrame->id);
    cppFrame.checksumModel = static_cast<ib::sim::lin::LinChecksumModel>(cFrame->checksumModel);
    cppFrame.dataLength = static_cast<ib::sim::lin::LinDataLengthT>(cFrame->dataLength);
    memcpy(cppFrame.data.data(), cFrame->data, 8);
}

static void assign(ib::sim::lin::LinFrameResponse& cppFrameResponse, const ib_Lin_FrameResponse* cFrameResponse)
{
    assign(cppFrameResponse.frame, cFrameResponse->frame);
    cppFrameResponse.responseMode = static_cast<ib::sim::lin::LinFrameResponseMode>(cFrameResponse->responseMode);
}

static void assign(std::vector<ib::sim::lin::LinFrameResponse>& cppFrameResponses,
                   const ib_Lin_FrameResponse* cFrameResponses, uint32_t numFrameResponses)
{
    for (uint32_t i = 0; i < numFrameResponses; i++)
    {
        ib::sim::lin::LinFrameResponse frameResponse;
        assign(frameResponse, &cFrameResponses[i]);
        cppFrameResponses.push_back(std::move(frameResponse));
    }
}

static void assign(ib::sim::lin::LinControllerConfig& cppConfig, const ib_Lin_ControllerConfig* cConfig)
{
    cppConfig.baudRate = static_cast<ib::sim::lin::LinBaudRateT>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<ib::sim::lin::LinControllerMode>(cConfig->controllerMode);
    assign(cppConfig.frameResponses, cConfig->frameResponses, cConfig->numFrameResponses);
}


extern "C" {

ib_ReturnCode ib_Lin_Controller_Create(ib_Lin_Controller** outLinController, ib_Participant* participant, const char* name, const char* network)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outLinController);
    ASSERT_VALID_POINTER_PARAMETER(name);
    ASSERT_VALID_POINTER_PARAMETER(network);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto cppLinController = cppParticipant->CreateLinController(name, network);
        if (cppLinController == nullptr)
        {
            return ib_ReturnCode_UNSPECIFIEDERROR;
        }
        *outLinController = reinterpret_cast<ib_Lin_Controller*>(cppLinController);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_Init(ib_Lin_Controller* controller, const ib_Lin_ControllerConfig* config) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        auto cppControllerConfig = ib::sim::lin::LinControllerConfig{};
        assign(cppControllerConfig, config);
        linController->Init(cppControllerConfig);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_Status(ib_Lin_Controller* controller, ib_Lin_ControllerStatus* outStatus)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_OUT_PARAMETER(outStatus);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        *outStatus = static_cast<ib_Lin_ControllerStatus>(linController->Status());
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SendFrame(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                                         ib_Lin_FrameResponseType responseType)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        ib::sim::lin::LinFrame cppFrame;
        assign(cppFrame, frame);
        linController->SendFrame(cppFrame, static_cast<ib::sim::lin::LinFrameResponseType>(responseType));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SendFrameHeader(ib_Lin_Controller* controller, ib_Lin_Id linId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->SendFrameHeader(static_cast<ib::sim::lin::LinIdT>(linId));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SetFrameResponse(ib_Lin_Controller* controller,
                                                 const ib_Lin_FrameResponse* frameResponse)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frameResponse);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        ib::sim::lin::LinFrame cppFrame;
        assign(cppFrame, frameResponse->frame);
        linController->SetFrameResponse(cppFrame,
                                        static_cast<ib::sim::lin::LinFrameResponseMode>(frameResponse->responseMode));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SetFrameResponses(ib_Lin_Controller* controller,
                                                  const ib_Lin_FrameResponse* frameResponses,
                                                  uint32_t numFrameResponses)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frameResponses);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        std::vector<ib::sim::lin::LinFrameResponse> cppFrameResponses;
        assign(cppFrameResponses, frameResponses, numFrameResponses);
        linController->SetFrameResponses(std::move(cppFrameResponses));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_GoToSleep(ib_Lin_Controller* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->GoToSleep();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_GoToSleepInternal(ib_Lin_Controller* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->GoToSleepInternal();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_Wakeup(ib_Lin_Controller* controller) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->Wakeup();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_WakeupInternal(ib_Lin_Controller* controller) 
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->WakeupInternal();
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_AddFrameStatusHandler(ib_Lin_Controller* controller, void* context,
                                                          ib_Lin_FrameStatusHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->AddFrameStatusHandler(
            [handler, context, controller](
                ib::sim::lin::ILinController* /*ctrl*/, const ib::sim::lin::LinFrameStatusEvent& cppFrameStatusEvent) 
            {
                ib_Lin_Frame cFrame;
                cFrame.id = static_cast<ib_Lin_Id>(cppFrameStatusEvent.frame.id);
                cFrame.checksumModel = static_cast<ib_Lin_ChecksumModel>(cppFrameStatusEvent.frame.checksumModel);
                cFrame.dataLength = static_cast<ib_Lin_DataLength>(cppFrameStatusEvent.frame.dataLength);
                memcpy(cFrame.data, cppFrameStatusEvent.frame.data.data(), 8);

                ib_Lin_FrameStatusEvent cFrameStatusEvent{};
                cFrameStatusEvent.interfaceId = ib_InterfaceIdentifier_LinFrameStatusEvent;
                cFrameStatusEvent.timestamp = (ib_NanosecondsTime)cppFrameStatusEvent.timestamp.count();
                cFrameStatusEvent.frame = &cFrame;
                cFrameStatusEvent.status = (ib_Lin_FrameStatus)cppFrameStatusEvent.status;

                handler(context, controller, &cFrameStatusEvent);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_AddGoToSleepHandler(ib_Lin_Controller* controller, void* context,
                                                        ib_Lin_GoToSleepHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->AddGoToSleepHandler(
            [handler, context, controller](ib::sim::lin::ILinController* /*ctrl*/,
                                           const ib::sim::lin::LinGoToSleepEvent& cppGoToSleepEvent) {
                ib_Lin_GoToSleepEvent goToSleepEvent{ib_InterfaceIdentifier_LinGoToSleepEvent,
                                                     (ib_NanosecondsTime)cppGoToSleepEvent.timestamp.count()};
                handler(context, controller, &goToSleepEvent);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_AddWakeupHandler(ib_Lin_Controller* controller, void* context,
                                                     ib_Lin_WakeupHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->AddWakeupHandler(
            [handler, context, controller](ib::sim::lin::ILinController* /*ctrl*/,
                                           const ib::sim::lin::LinWakeupEvent& cppWakeupEvent) {
                ib_Lin_WakeupEvent wakeupEvent{ib_InterfaceIdentifier_LinWakeupEvent,
                                               (ib_NanosecondsTime)cppWakeupEvent.timestamp.count(),
                                               (ib_Direction)cppWakeupEvent.direction};
                handler(context, controller, &wakeupEvent);
        });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
