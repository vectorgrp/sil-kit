/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include <sstream>

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "CapiImpl.hpp"
#include "silkit/services/can/all.hpp"

struct PendingTransmits
{
    std::map<uint32_t, void*> userContextById;
    std::map<uint32_t, std::function<void()>> callbacksById;
    std::mutex mutex;
};
PendingTransmits pendingTransmits;

SilKit_ReturnCode SilKit_CanController_Create(SilKit_CanController** outController, SilKit_Participant* participant,
        const char* cName, const char* cNetwork)
{
    ASSERT_VALID_OUT_PARAMETER(outController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(cName);
    ASSERT_VALID_POINTER_PARAMETER(cNetwork);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::Core::IParticipant*>(participant);
        auto canController = cppParticipant->CreateCanController(cName, cNetwork);
        *outController = reinterpret_cast<SilKit_CanController*>(canController);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_AddFrameHandler(SilKit_CanController* controller, void* context,
        SilKit_CanFrameHandler_t callback, SilKit_Direction directionMask,
        SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        *outHandlerId = (SilKit_HandlerId)canController->AddFrameHandler(
                [context, controller, callback](SilKit::Services::Can::ICanController* /*ctrl*/,
                    const SilKit::Services::Can::CanFrameEvent& cppCanFrameEvent) {
                SilKit_CanFrame frame{};
                frame.id = cppCanFrameEvent.frame.canId;
                uint32_t flags = 0;
                flags |= cppCanFrameEvent.frame.flags.ide ? SilKit_CanFrameFlag_ide : 0;
                flags |= cppCanFrameEvent.frame.flags.rtr ? SilKit_CanFrameFlag_rtr : 0;
                flags |= cppCanFrameEvent.frame.flags.fdf ? SilKit_CanFrameFlag_fdf : 0;
                flags |= cppCanFrameEvent.frame.flags.brs ? SilKit_CanFrameFlag_brs : 0;
                flags |= cppCanFrameEvent.frame.flags.esi ? SilKit_CanFrameFlag_esi : 0;
                frame.flags = flags;
                frame.dlc = cppCanFrameEvent.frame.dlc;
                frame.data = {
                    (uint8_t*)cppCanFrameEvent.frame.dataField.data(),
                    (uint32_t)cppCanFrameEvent.frame.dataField.size()
                };

                SilKit_CanFrameEvent frameEvent{};
                frameEvent.interfaceId = SilKit_InterfaceIdentifier_CanFrameEvent;
                frameEvent.timestamp = cppCanFrameEvent.timestamp.count();
                frameEvent.interfaceId = SilKit_InterfaceIdentifier_CanFrameEvent;
                frameEvent.frame = &frame;
                frameEvent.userContext = cppCanFrameEvent.userContext;

                callback(context, controller, &frameEvent);
        }, directionMask);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_RemoveFrameHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->RemoveFrameHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_AddFrameTransmitHandler(SilKit_CanController* controller, void* context,
        SilKit_CanFrameTransmitHandler_t callback,
        SilKit_CanTransmitStatus statusMask, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        *outHandlerId = (SilKit_HandlerId)canController->AddFrameTransmitHandler(
                [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                    const SilKit::Services::Can::CanFrameTransmitEvent& cppFrameTransmitEvent) {
                SilKit_CanFrameTransmitEvent frameTransmitEvent{};
                frameTransmitEvent.interfaceId = SilKit_InterfaceIdentifier_CanFrameTransmitEvent;
                frameTransmitEvent.userContext = cppFrameTransmitEvent.userContext;
                frameTransmitEvent.timestamp = cppFrameTransmitEvent.timestamp.count();
                frameTransmitEvent.status = (SilKit_CanTransmitStatus)cppFrameTransmitEvent.status;
                callback(context, controller, &frameTransmitEvent);
                },
                static_cast<SilKit::Services::Can::CanTransmitStatusMask>(statusMask));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_RemoveFrameTransmitHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->RemoveFrameTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_AddStateChangeHandler(SilKit_CanController* controller, void* context,
        SilKit_CanStateChangeHandler_t callback, SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        *outHandlerId = (SilKit_HandlerId)canController->AddStateChangeHandler(
                [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                    const SilKit::Services::Can::CanStateChangeEvent cppStateChangeEvent) {
                SilKit_CanStateChangeEvent stateChangeEvent;
                stateChangeEvent.interfaceId = SilKit_InterfaceIdentifier_CanStateChangeEvent;
                stateChangeEvent.timestamp = cppStateChangeEvent.timestamp.count();
                stateChangeEvent.state = (SilKit_CanControllerState)cppStateChangeEvent.state;
                callback(context, controller, &stateChangeEvent);
                });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_RemoveStateChangeHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->RemoveStateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_AddErrorStateChangeHandler(SilKit_CanController* controller, void* context,
        SilKit_CanErrorStateChangeHandler_t callback,
        SilKit_HandlerId* outHandlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        auto cppHandlerId = canController->AddErrorStateChangeHandler(
                [callback, context, controller](SilKit::Services::Can::ICanController* /*ctrl*/,
                    const SilKit::Services::Can::CanErrorStateChangeEvent cppErrorStateChangeEvent) {
                SilKit_CanErrorStateChangeEvent errorStateChangeEvent;
                errorStateChangeEvent.interfaceId = SilKit_InterfaceIdentifier_CanErrorStateChangeEvent;
                errorStateChangeEvent.timestamp = cppErrorStateChangeEvent.timestamp.count();
                errorStateChangeEvent.errorState = (SilKit_CanErrorState)cppErrorStateChangeEvent.errorState;
                callback(context, controller, &errorStateChangeEvent);
                });
        *outHandlerId = static_cast<SilKit_HandlerId>(cppHandlerId);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_RemoveErrorStateChangeHandler(SilKit_CanController* controller, SilKit_HandlerId handlerId)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->RemoveErrorStateChangeHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_SetBaudRate(SilKit_CanController* controller, uint32_t rate, uint32_t fdRate)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->SetBaudRate(rate, fdRate);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_SendFrame(SilKit_CanController* controller, SilKit_CanFrame* message, void* transmitContext)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(message);
    CAPI_ENTER
    {
        using std::chrono::duration;
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);

        SilKit::Services::Can::CanFrame frame{};
        frame.canId = message->id;
        frame.flags.ide = message->flags & SilKit_CanFrameFlag_ide ? 1 : 0;
        frame.flags.rtr = message->flags & SilKit_CanFrameFlag_rtr ? 1 : 0;
        frame.flags.fdf = message->flags & SilKit_CanFrameFlag_fdf ? 1 : 0;
        frame.flags.brs = message->flags & SilKit_CanFrameFlag_brs ? 1 : 0;
        frame.flags.esi = message->flags & SilKit_CanFrameFlag_esi ? 1 : 0;

        frame.dlc = message->dlc;
        frame.dataField = std::vector<uint8_t>{message->data.data,  message->data.data + message->data.size};

        canController->SendFrame(std::move(frame), transmitContext);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_Start(SilKit_CanController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->Start();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_Stop(SilKit_CanController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->Stop();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_Reset(SilKit_CanController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->Reset();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_CanController_Sleep(SilKit_CanController* controller)
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    CAPI_ENTER
    {
        auto canController = reinterpret_cast<SilKit::Services::Can::ICanController*>(controller);
        canController->Sleep();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

