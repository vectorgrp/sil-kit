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

#include <map>
#include <mutex>
#include <cstring>
#include <sstream>

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "CapiImpl.hpp"
#include "silkit/services/can/all.hpp"


SilKit_ReturnCode SilKitCALL SilKit_CanController_Create(SilKit_CanController** outController, SilKit_Participant* participant,
        const char* cName, const char* cNetwork)
try
{
    ASSERT_VALID_OUT_PARAMETER(outController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(cName);
    ASSERT_VALID_POINTER_PARAMETER(cNetwork);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto canController = cppParticipant->CreateCanController(cName, cNetwork);
    *outController = reinterpret_cast<SilKit_CanController*>(canController);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameHandler(SilKit_CanController* controller, void* context,
        SilKit_CanFrameHandler_t callback, SilKit_Direction directionMask,
        SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    *outHandlerId = (SilKit_HandlerId)canController->AddFrameHandler(
            [context, controller, callback](SilKit::Services::Can::ICanController* /*ctrl*/,
                const SilKit::Services::Can::CanFrameEvent& cppCanFrameEvent) {
            SilKit_CanFrame frame{};
            SilKit_Struct_Init(SilKit_CanFrame, frame);
            frame.id = cppCanFrameEvent.frame.canId;
            frame.flags = cppCanFrameEvent.frame.flags;
            frame.dlc = cppCanFrameEvent.frame.dlc;
            frame.sdt = cppCanFrameEvent.frame.sdt;
            frame.vcid = cppCanFrameEvent.frame.vcid;
            frame.af = cppCanFrameEvent.frame.af;
            frame.data = ToSilKitByteVector(cppCanFrameEvent.frame.dataField);

            SilKit_CanFrameEvent frameEvent{};
            SilKit_Struct_Init(SilKit_CanFrameEvent, frameEvent);
            frameEvent.timestamp = cppCanFrameEvent.timestamp.count();
            frameEvent.frame = &frame;
            frameEvent.direction = static_cast<SilKit_Direction>(cppCanFrameEvent.direction);
            frameEvent.userContext = cppCanFrameEvent.userContext;

            callback(context, controller, &frameEvent);
    }, directionMask);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->RemoveFrameHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameTransmitHandler(SilKit_CanController* controller, void* context,
        SilKit_CanFrameTransmitHandler_t callback,
        SilKit_CanTransmitStatus statusMask, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    *outHandlerId = (SilKit_HandlerId)canController->AddFrameTransmitHandler(
            [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                const SilKit::Services::Can::CanFrameTransmitEvent& cppFrameTransmitEvent) {
            SilKit_CanFrameTransmitEvent frameTransmitEvent{};
            SilKit_Struct_Init(SilKit_CanFrameTransmitEvent, frameTransmitEvent);
            frameTransmitEvent.userContext = cppFrameTransmitEvent.userContext;
            frameTransmitEvent.timestamp = cppFrameTransmitEvent.timestamp.count();
            frameTransmitEvent.status = (SilKit_CanTransmitStatus)cppFrameTransmitEvent.status;
            frameTransmitEvent.canId = cppFrameTransmitEvent.canId;
            callback(context, controller, &frameTransmitEvent);
            },
            static_cast<SilKit::Services::Can::CanTransmitStatusMask>(statusMask));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameTransmitHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->RemoveFrameTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_AddStateChangeHandler(SilKit_CanController* controller, void* context,
        SilKit_CanStateChangeHandler_t callback, SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    *outHandlerId = (SilKit_HandlerId)canController->AddStateChangeHandler(
            [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                const SilKit::Services::Can::CanStateChangeEvent cppStateChangeEvent) {
            SilKit_CanStateChangeEvent stateChangeEvent;
            SilKit_Struct_Init(SilKit_CanStateChangeEvent, stateChangeEvent);
            stateChangeEvent.timestamp = cppStateChangeEvent.timestamp.count();
            stateChangeEvent.state = (SilKit_CanControllerState)cppStateChangeEvent.state;
            callback(context, controller, &stateChangeEvent);
            });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveStateChangeHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->RemoveStateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_AddErrorStateChangeHandler(SilKit_CanController* controller, void* context,
        SilKit_CanErrorStateChangeHandler_t callback,
        SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    auto cppHandlerId = canController->AddErrorStateChangeHandler(
            [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                const SilKit::Services::Can::CanErrorStateChangeEvent cppErrorStateChangeEvent) {
            SilKit_CanErrorStateChangeEvent errorStateChangeEvent;
            SilKit_Struct_Init(SilKit_CanErrorStateChangeEvent, errorStateChangeEvent);
            errorStateChangeEvent.timestamp = cppErrorStateChangeEvent.timestamp.count();
            errorStateChangeEvent.errorState = (SilKit_CanErrorState)cppErrorStateChangeEvent.errorState;
            callback(context, controller, &errorStateChangeEvent);
            });
    *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveErrorStateChangeHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->RemoveErrorStateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_SetBaudRate(SilKit_CanController* controller, uint32_t rate, uint32_t fdRate, uint32_t xlRate)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->SetBaudRate(rate, fdRate, xlRate);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_SendFrame(SilKit_CanController* controller, SilKit_CanFrame* message, void* transmitContext)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(message);
    ASSERT_VALID_STRUCT_HEADER(message);

    using std::chrono::duration;
    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);

    SilKit::Services::Can::CanFrame frame{};
    frame.canId = message->id;
    frame.flags = message->flags;
    frame.dlc = message->dlc;
    frame.sdt = message->sdt;
    frame.vcid = message->vcid;
    frame.af = message->af;
    frame.dataField = SilKit::Util::ToSpan(message->data);

    canController->SendFrame(std::move(frame), transmitContext);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_Start(SilKit_CanController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->Start();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_Stop(SilKit_CanController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->Stop();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_Reset(SilKit_CanController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->Reset();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_CanController_Sleep(SilKit_CanController* controller)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
    canController->Sleep();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
