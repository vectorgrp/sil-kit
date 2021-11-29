/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/sim/lin/all.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "capiImpl.h"
#include <cstring>

static void assign(ib::sim::lin::Frame& cppFrame, const ib_LinFrame* cFrame) 
{
    cppFrame.id = static_cast<ib::sim::lin::LinIdT>(cFrame->id);
    cppFrame.checksumModel = static_cast<ib::sim::lin::ChecksumModel>(cFrame->checksumModel);
    cppFrame.dataLength = static_cast<ib::sim::lin::DataLengthT>(cFrame->dataLength);
    memcpy(cppFrame.data.data(), cFrame->data, 8);
}

static void assign(ib::sim::lin::FrameResponse& cppFrameResponse, const ib_LinFrameResponse* cFrameResponse)
{
    assign(cppFrameResponse.frame, &cFrameResponse->frame);
    cppFrameResponse.responseMode = static_cast<ib::sim::lin::FrameResponseMode>(cFrameResponse->responseMode);
}

static void assign(std::vector<ib::sim::lin::FrameResponse>& cppFrameResponses,
                   const ib_LinFrameResponse* cFrameResponses, uint32_t numFrameResponses)
{
    for (uint32_t i = 0; i < numFrameResponses; i++)
    {
        ib::sim::lin::FrameResponse frameResponse;
        assign(frameResponse, &cFrameResponses[i]);
        cppFrameResponses.push_back(std::move(frameResponse));
    }
}

static void assign(ib::sim::lin::ControllerConfig& cppConfig, const ib_LinControllerConfig* cConfig)
{
    cppConfig.baudRate = static_cast<ib::sim::lin::BaudRateT>(cConfig->baudRate);
    cppConfig.controllerMode = static_cast<ib::sim::lin::ControllerMode>(cConfig->controllerMode);
    assign(cppConfig.frameResponses, cConfig->frameResponses, cConfig->numFrameResponses);
}


extern "C" {

ib_ReturnCode ib_LinController_create(ib_LinController** outLinController, ib_SimulationParticipant* participant, 
                                      const char* name)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_OUT_PARAMETER(outLinController);
    ASSERT_VALID_POINTER_PARAMETER(name);
    CAPI_ENTER
    {
        std::string name(name);
        auto        comAdapter = reinterpret_cast<ib::mw::IComAdapter*>(participant);
        auto        cppLinController = comAdapter->CreateLinController(name);
        if (cppLinController == nullptr)
        {
            return ib_ReturnCode_UNSPECIFIEDERROR;
        }
        *outLinController = reinterpret_cast<ib_LinController*>(cppLinController);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_LinController_Init(ib_LinController* controller, const ib_LinControllerConfig* config) 
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

ib_ReturnCode ib_LinController_Status(ib_LinController* controller, ib_LinControllerStatus* outStatus)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_OUT_PARAMETER(outStatus);
    CAPI_ENTER
    {
        auto linController = reinterpret_cast<ib::sim::lin::ILinController*>(controller);
        *outStatus = static_cast<ib_LinControllerStatus>(linController->Status());
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_LinController_SendFrame(ib_LinController* controller, const ib_LinFrame* frame,
                                         ib_LinFrameResponseType responseType)
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

ib_ReturnCode ib_LinController_SendFrameWithTimestamp(ib_LinController* controller, const ib_LinFrame* frame,
                                                      ib_LinFrameResponseType responseType,
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


ib_ReturnCode ib_LinController_SendFrameHeader(ib_LinController* controller, ib_LinId linId)
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

ib_ReturnCode ib_LinController_SendFrameHeaderWithTimestamp(ib_LinController* controller, ib_LinId linId,
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

ib_ReturnCode ib_LinController_SetFrameResponse(ib_LinController* controller, const ib_LinFrame* frame,
                                                                   ib_LinFrameResponseMode mode)
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


ib_ReturnCode ib_LinController_SetFrameResponses(ib_LinController* controller, const ib_LinFrameResponse* frameResponses,
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

ib_ReturnCode ib_LinController_GoToSleep(ib_LinController* controller)
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

ib_ReturnCode ib_LinController_GoToSleepInternal(ib_LinController* controller)
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

ib_ReturnCode ib_LinController_Wakeup(ib_LinController* controller) 
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

ib_ReturnCode ib_LinController_WakeupInternal(ib_LinController* controller) 
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

ib_ReturnCode ib_LinController_RegisterFrameStatusHandler(ib_LinController* controller, void* context,
                                                          ib_LinFrameStatusHandler_t handler)
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
                ib_LinFrame cFrame;
                cFrame.id = static_cast<ib_LinId>(cppFrame.id);
                cFrame.checksumModel = static_cast<ib_LinChecksumModel>(cppFrame.checksumModel);
                cFrame.dataLength = static_cast<ib_LinDataLength>(cppFrame.dataLength);
                memcpy(cFrame.data, cppFrame.data.data(), 8);

                handler(context, controller, &cFrame, static_cast<ib_LinFrameStatus>(cppFrameStatus),
                        static_cast<ib_NanosecondsTime>(cppTimestamp.count()));
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_LinController_RegisterGoToSleepHandler(ib_LinController* controller, void* context,
                                                        ib_LinGoToSleepHandler_t handler)
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

ib_ReturnCode ib_LinController_RegisterWakeupHandler(ib_LinController* controller, void* context,
                                                     ib_LinWakeupHandler_t handler)
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
