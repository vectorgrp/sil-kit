/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "CapiImpl.h"
#include <cstring>

static void assign(ib::sim::lin::Frame& cppFrame, const ib_Lin_Frame* cFrame) 
{
    cppFrame.id = static_cast<ib::sim::lin::LinIdT>(cFrame->id);
    cppFrame.checksumModel = static_cast<ib::sim::lin::ChecksumModel>(cFrame->checksumModel);
    cppFrame.dataLength = static_cast<ib::sim::lin::DataLengthT>(cFrame->dataLength);
    memcpy(cppFrame.data.data(), cFrame->data, 8);
}

static void assign(ib::sim::lin::FrameResponse& cppFrameResponse, const ib_Lin_FrameResponse* cFrameResponse)
{
    assign(cppFrameResponse.frame, &cFrameResponse->frame);
    cppFrameResponse.responseMode = static_cast<ib::sim::lin::FrameResponseMode>(cFrameResponse->responseMode);
}

static void assign(std::vector<ib::sim::lin::FrameResponse>& cppFrameResponses,
                   const ib_Lin_FrameResponse* cFrameResponses, uint32_t numFrameResponses)
{
    for (uint32_t i = 0; i < numFrameResponses; i++)
    {
        ib::sim::lin::FrameResponse frameResponse;
        assign(frameResponse, &cFrameResponses[i]);
        cppFrameResponses.push_back(std::move(frameResponse));
    }
}

static void assign(ib::sim::lin::ControllerConfig& cppConfig, const ib_Lin_ControllerConfig* cConfig)
{
    cppConfig.baudRate = static_cast<ib::sim::lin::BaudRateT>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<ib::sim::lin::ControllerMode>(cConfig->controllerMode);
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
        auto cppControllerConfig = ib::sim::lin::ControllerConfig{};
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
        ib::sim::lin::Frame cppFrame;
        assign(cppFrame, frame);
        linController->SendFrame(cppFrame, static_cast<ib::sim::lin::FrameResponseType>(responseType));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SendFrameWithTimestamp(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                                                      ib_Lin_FrameResponseType responseType,
                                                      ib_NanosecondsTime      timestamp)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        ib::sim::lin::Frame cppFrame;
        assign(cppFrame, frame);
        linController->SendFrame(cppFrame, static_cast<ib::sim::lin::FrameResponseType>(responseType), std::chrono::nanoseconds(timestamp));
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

ib_ReturnCode ib_Lin_Controller_SendFrameHeaderWithTimestamp(ib_Lin_Controller* controller, ib_Lin_Id linId,
                                                            ib_NanosecondsTime timestamp)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->SendFrameHeader(static_cast<ib::sim::lin::LinIdT>(linId), std::chrono::nanoseconds(timestamp));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_SetFrameResponse(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                                                                   ib_Lin_FrameResponseMode mode)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        ib::sim::lin::Frame cppFrame;
        assign(cppFrame, frame);
        linController->SetFrameResponse(cppFrame, static_cast<ib::sim::lin::FrameResponseMode>(mode));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}


ib_ReturnCode ib_Lin_Controller_SetFrameResponses(ib_Lin_Controller* controller, const ib_Lin_FrameResponse* frameResponses,
                                                 uint32_t numFrameResponses)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frameResponses);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        std::vector<ib::sim::lin::FrameResponse> cppFrameResponses;
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

ib_ReturnCode ib_Lin_Controller_RegisterFrameStatusHandler(ib_Lin_Controller* controller, void* context,
                                                          ib_Lin_FrameStatusHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->RegisterFrameStatusHandler(
            [handler, context, controller](
                ib::sim::lin::ILinController* /*ctrl*/, const ib::sim::lin::Frame& cppFrame,
                ib::sim::lin::FrameStatus cppFrameStatus, std::chrono::nanoseconds cppTimestamp) 
            {
                ib_Lin_Frame cFrame;
                cFrame.id = static_cast<ib_Lin_Id>(cppFrame.id);
                cFrame.checksumModel = static_cast<ib_Lin_ChecksumModel>(cppFrame.checksumModel);
                cFrame.dataLength = static_cast<ib_Lin_DataLength>(cppFrame.dataLength);
                memcpy(cFrame.data, cppFrame.data.data(), 8);

                handler(context, controller, &cFrame, static_cast<ib_Lin_FrameStatus>(cppFrameStatus),
                        static_cast<ib_NanosecondsTime>(cppTimestamp.count()));
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_RegisterGoToSleepHandler(ib_Lin_Controller* controller, void* context,
                                                        ib_Lin_GoToSleepHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->RegisterGoToSleepHandler(
            [handler, context, controller](ib::sim::lin::ILinController* /*ctrl*/) { handler(context, controller); });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Lin_Controller_RegisterWakeupHandler(ib_Lin_Controller* controller, void* context,
                                                     ib_Lin_WakeupHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        linController->RegisterWakeupHandler(
            [handler, context, controller](ib::sim::lin::ILinController* /*ctrl*/) { handler(context, controller); });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

}
