// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <cstring>

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "CapiImpl.hpp"
#include "silkit/services/i2c/all.hpp"


SilKit_ReturnCode SilKitCALL SilKit_I2cController_Create(SilKit_I2cController** outI2cController,
                                                          SilKit_Participant* participant, const char* cName,
                                                          const char* cNetwork)
try
{
    ASSERT_VALID_OUT_PARAMETER(outI2cController);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(cName);
    ASSERT_VALID_POINTER_PARAMETER(cNetwork);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    auto i2cController = cppParticipant->CreateI2cController(cName, cNetwork);
    *outI2cController = reinterpret_cast<SilKit_I2cController*>(i2cController);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_Init(SilKit_I2cController* controller,
                                                        SilKit_I2cControllerConfig* config)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(config);
    ASSERT_VALID_STRUCT_HEADER(config);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);

    SilKit::Services::I2c::I2cControllerConfig cppConfig{};
    cppConfig.mode = static_cast<SilKit::Services::I2c::I2cControllerMode>(config->mode);
    cppConfig.slaveAddress = config->slaveAddress;
    cppConfig.slaveAddressMode = static_cast<SilKit::Services::I2c::I2cAddressMode>(config->slaveAddressMode);
    cppConfig.speedMode = static_cast<SilKit::Services::I2c::I2cSpeedMode>(config->speedMode);

    cppController->Init(std::move(cppConfig));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_SendFrame(SilKit_I2cController* controller, SilKit_I2cFrame* frame,
                                                             void* userContext)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(frame);
    ASSERT_VALID_STRUCT_HEADER(frame);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);

    SilKit::Services::I2c::I2cFrame cppFrame{};
    cppFrame.address = frame->address;
    cppFrame.addressMode = static_cast<SilKit::Services::I2c::I2cAddressMode>(frame->addressMode);
    cppFrame.direction = static_cast<SilKit::Services::I2c::I2cTransferDirection>(frame->direction);
    cppFrame.data = SilKit::Util::ToSpan(frame->data);

    cppController->SendFrame(cppFrame, userContext);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_SetReadResponse(SilKit_I2cController* controller,
                                                                   SilKit_ByteVector* data)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_POINTER_PARAMETER(data);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);
    cppController->SetReadResponse(SilKit::Util::ToSpan(*data));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_AddFrameHandler(SilKit_I2cController* controller, void* context,
                                                                   SilKit_I2cFrameHandler_t callback,
                                                                   SilKit_Direction directionMask,
                                                                   SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddFrameHandler(
        [context, controller, callback](SilKit::Services::I2c::II2cController* /*ctrl*/,
                                        const SilKit::Services::I2c::I2cFrameEvent& cppEvent) {
        SilKit_I2cFrame frame{};
        SilKit_Struct_Init(SilKit_I2cFrame, frame);
        frame.address = cppEvent.frame.address;
        frame.addressMode = static_cast<SilKit_I2cAddressMode>(cppEvent.frame.addressMode);
        frame.direction = static_cast<SilKit_I2cTransferDirection>(cppEvent.frame.direction);
        frame.data = SilKit::Util::ToSilKitByteVector(cppEvent.frame.data);

        SilKit_I2cFrameEvent frameEvent{};
        SilKit_Struct_Init(SilKit_I2cFrameEvent, frameEvent);
        frameEvent.timestamp = cppEvent.timestamp.count();
        frameEvent.frame = &frame;
        frameEvent.direction = static_cast<SilKit_Direction>(cppEvent.direction);
        frameEvent.userContext = cppEvent.userContext;

        callback(context, controller, &frameEvent);
    },
        static_cast<SilKit::Services::DirectionMask>(directionMask));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_RemoveFrameHandler(SilKit_I2cController* controller,
                                                                      SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);
    cppController->RemoveFrameHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_AddFrameTransmitHandler(
    SilKit_I2cController* controller, void* context, SilKit_I2cFrameTransmitHandler_t callback,
    SilKit_HandlerId* outHandlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);
    ASSERT_VALID_HANDLER_PARAMETER(callback);
    ASSERT_VALID_OUT_PARAMETER(outHandlerId);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);
    *outHandlerId = (SilKit_HandlerId)cppController->AddFrameTransmitHandler(
        [callback, context, controller](SilKit::Services::I2c::II2cController* /*ctrl*/,
                                        const SilKit::Services::I2c::I2cFrameTransmitEvent& cppEvent) {
        SilKit_I2cFrameTransmitEvent transmitEvent{};
        SilKit_Struct_Init(SilKit_I2cFrameTransmitEvent, transmitEvent);
        transmitEvent.timestamp = cppEvent.timestamp.count();
        transmitEvent.address = cppEvent.address;
        transmitEvent.status = static_cast<SilKit_I2cTransmitStatus>(cppEvent.status);
        transmitEvent.userContext = cppEvent.userContext;
        callback(context, controller, &transmitEvent);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_I2cController_RemoveFrameTransmitHandler(SilKit_I2cController* controller,
                                                                              SilKit_HandlerId handlerId)
try
{
    ASSERT_VALID_POINTER_PARAMETER(controller);

    auto cppController = reinterpret_cast<SilKit::Services::I2c::II2cController*>(controller);
    cppController->RemoveFrameTransmitHandler(static_cast<SilKit::Util::HandlerId>(handlerId));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
