// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>

#include "silkit/capi/I2c.h"

#include "silkit/services/i2c/II2cController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace I2c {

class I2cController : public SilKit::Services::I2c::II2cController
{
public:
    inline I2cController(SilKit_Participant* participant, const std::string& canonicalName,
                         const std::string& networkName);

    inline ~I2cController() override = default;

    inline void Init(SilKit::Services::I2c::I2cControllerConfig config) override;

    inline void SendFrame(const SilKit::Services::I2c::I2cFrame& frame, void* userContext) override;

    inline void SetReadResponse(SilKit::Util::Span<const uint8_t> data) override;

    inline auto AddFrameHandler(FrameHandler handler,
                                SilKit::Services::DirectionMask directionMask) -> Util::HandlerId override;

    inline void RemoveFrameHandler(Util::HandlerId handlerId) override;

    inline auto AddFrameTransmitHandler(FrameTransmitHandler handler) -> Util::HandlerId override;

    inline void RemoveFrameTransmitHandler(Util::HandlerId handlerId) override;

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::I2c::II2cController* controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_I2cController* _i2cController{nullptr};

    HandlerDataMap<FrameHandler> _frameHandlers;
    HandlerDataMap<FrameTransmitHandler> _frameTransmitHandlers;
};

} // namespace I2c
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/capi/InterfaceIdentifiers.h"

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace I2c {

I2cController::I2cController(SilKit_Participant* participant, const std::string& canonicalName,
                             const std::string& networkName)
{
    const auto returnCode =
        SilKit_I2cController_Create(&_i2cController, participant, canonicalName.c_str(), networkName.c_str());
    ThrowOnError(returnCode);
}

void I2cController::Init(SilKit::Services::I2c::I2cControllerConfig config)
{
    SilKit_I2cControllerConfig cConfig{};
    SilKit_Struct_Init(SilKit_I2cControllerConfig, cConfig);
    cConfig.mode = static_cast<SilKit_I2cControllerMode>(config.mode);
    cConfig.slaveAddress = config.slaveAddress;
    cConfig.slaveAddressMode = static_cast<SilKit_I2cAddressMode>(config.slaveAddressMode);
    cConfig.speedMode = static_cast<SilKit_I2cSpeedMode>(config.speedMode);
    const auto returnCode = SilKit_I2cController_Init(_i2cController, &cConfig);
    ThrowOnError(returnCode);
}

void I2cController::SendFrame(const SilKit::Services::I2c::I2cFrame& frame, void* userContext)
{
    SilKit_I2cFrame cFrame{};
    SilKit_Struct_Init(SilKit_I2cFrame, cFrame);
    cFrame.address = frame.address;
    cFrame.addressMode = static_cast<SilKit_I2cAddressMode>(frame.addressMode);
    cFrame.direction = static_cast<SilKit_I2cTransferDirection>(frame.direction);
    cFrame.data = SilKit::Util::ToSilKitByteVector(frame.data);
    const auto returnCode = SilKit_I2cController_SendFrame(_i2cController, &cFrame, userContext);
    ThrowOnError(returnCode);
}

void I2cController::SetReadResponse(SilKit::Util::Span<const uint8_t> data)
{
    auto cData = SilKit::Util::ToSilKitByteVector(data);
    const auto returnCode = SilKit_I2cController_SetReadResponse(_i2cController, &cData);
    ThrowOnError(returnCode);
}

auto I2cController::AddFrameHandler(FrameHandler handler,
                                    SilKit::Services::DirectionMask directionMask) -> Util::HandlerId
{
    const auto cHandler = [](void* context, SilKit_I2cController* controller,
                             SilKit_I2cFrameEvent* frameEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::I2c::I2cFrame frame{};
        frame.address = frameEvent->frame->address;
        frame.addressMode = static_cast<SilKit::Services::I2c::I2cAddressMode>(frameEvent->frame->addressMode);
        frame.direction = static_cast<SilKit::Services::I2c::I2cTransferDirection>(frameEvent->frame->direction);
        frame.data = SilKit::Util::ToSpan(frameEvent->frame->data);

        SilKit::Services::I2c::I2cFrameEvent event{};
        event.timestamp = std::chrono::nanoseconds{frameEvent->timestamp};
        event.frame = frame;
        event.direction = static_cast<SilKit::Services::TransmitDirection>(frameEvent->direction);
        event.userContext = frameEvent->userContext;

        const auto data = static_cast<HandlerData<FrameHandler>*>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_I2cController_AddFrameHandler(
        _i2cController, handlerData.get(), cHandler, static_cast<SilKit_Direction>(directionMask), &handlerId);
    ThrowOnError(returnCode);

    _frameHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void I2cController::RemoveFrameHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_I2cController_RemoveFrameHandler(_i2cController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameHandlers.erase(handlerId);
}

auto I2cController::AddFrameTransmitHandler(FrameTransmitHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void* context, SilKit_I2cController* controller,
                             SilKit_I2cFrameTransmitEvent* transmitEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::I2c::I2cFrameTransmitEvent event{};
        event.timestamp = std::chrono::nanoseconds{transmitEvent->timestamp};
        event.address = transmitEvent->address;
        event.status = static_cast<SilKit::Services::I2c::I2cTransmitStatus>(transmitEvent->status);
        event.userContext = transmitEvent->userContext;

        const auto data = static_cast<HandlerData<FrameTransmitHandler>*>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameTransmitHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_I2cController_AddFrameTransmitHandler(_i2cController, handlerData.get(), cHandler,
                                                                         &handlerId);
    ThrowOnError(returnCode);

    _frameTransmitHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void I2cController::RemoveFrameTransmitHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_I2cController_RemoveFrameTransmitHandler(_i2cController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameTransmitHandlers.erase(handlerId);
}

} // namespace I2c
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
