// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <unordered_map>

#include "silkit/capi/Can.h"

#include "silkit/services/can/ICanController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Can {

class CanController : public SilKit::Services::Can::ICanController
{
public:
    inline CanController(SilKit_Participant *participant, const std::string &canonicalName,
                         const std::string &networkName);

    inline ~CanController() override = default;

    inline void SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate) override;

    inline void Reset() override;

    inline void Start() override;

    inline void Stop() override;

    inline void Sleep() override;

    inline void SendFrame(const SilKit::Services::Can::CanFrame &msg, void *userContext) override;

    inline auto AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
        -> Util::HandlerId override;

    inline void RemoveFrameHandler(Util::HandlerId handlerId) override;

    inline auto AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId override;

    inline void RemoveStateChangeHandler(Util::HandlerId handlerId) override;

    inline auto AddErrorStateChangeHandler(ErrorStateChangeHandler handler) -> Util::HandlerId override;

    inline void RemoveErrorStateChangeHandler(Util::HandlerId handlerId) override;

    inline auto AddFrameTransmitHandler(FrameTransmitHandler handler,
                                        SilKit::Services::Can::CanTransmitStatusMask statusMask)
        -> Util::HandlerId override;

    inline void RemoveFrameTransmitHandler(SilKit::Util::HandlerId handlerId) override;

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Can::ICanController *controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_CanController *_canController{nullptr};

    HandlerDataMap<FrameHandler> _frameHandlers;
    HandlerDataMap<StateChangeHandler> _stateChangeHandlers;
    HandlerDataMap<ErrorStateChangeHandler> _errorStateChangeHandlers;
    HandlerDataMap<FrameTransmitHandler> _frameTransmitHandlers;
};

} // namespace Can
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
namespace Can {

CanController::CanController(SilKit_Participant *participant, const std::string &canonicalName,
                             const std::string &networkName)
{
    const auto returnCode =
        SilKit_CanController_Create(&_canController, participant, canonicalName.c_str(), networkName.c_str());
    ThrowOnError(returnCode);
}

void CanController::SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate)
{
    const auto returnCode = SilKit_CanController_SetBaudRate(_canController, rate, fdRate, xlRate);
    ThrowOnError(returnCode);
}

void CanController::Reset()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_CanController_Reset(_canController);
    ThrowOnError(returnCode);
}

void CanController::Start()
{
    const auto returnCode = SilKit_CanController_Start(_canController);
    ThrowOnError(returnCode);
}

void CanController::Stop()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_CanController_Stop(_canController);
    ThrowOnError(returnCode);
}

void CanController::Sleep()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_CanController_Sleep(_canController);
    ThrowOnError(returnCode);
}

void CanController::SendFrame(const SilKit::Services::Can::CanFrame &msg, void *userContext)
{
    SilKit_CanFrame canFrame;
    SilKit_Struct_Init(SilKit_CanFrame, canFrame);
    canFrame.id = msg.canId;
    canFrame.flags = msg.flags;
    canFrame.dlc = msg.dlc;
    canFrame.sdt = msg.sdt;
    canFrame.vcid = msg.vcid;
    canFrame.af = msg.af;
    canFrame.data = ToSilKitByteVector(msg.dataField);

    const auto returnCode = SilKit_CanController_SendFrame(_canController, &canFrame, userContext);
    ThrowOnError(returnCode);
}

auto CanController::AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
    -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_CanController *controller, SilKit_CanFrameEvent *frameEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Can::CanFrameEvent event{};
        event.timestamp = std::chrono::nanoseconds{frameEvent->timestamp};
        event.frame.canId = frameEvent->frame->id;
        event.frame.flags = frameEvent->frame->flags;
        event.frame.dlc = frameEvent->frame->dlc;
        event.frame.sdt = frameEvent->frame->sdt;
        event.frame.vcid = frameEvent->frame->vcid;
        event.frame.af = frameEvent->frame->af;
        event.frame.dataField = SilKit::Util::ToSpan(frameEvent->frame->data);
        event.direction = static_cast<SilKit::Services::TransmitDirection>(frameEvent->direction);
        event.userContext = frameEvent->userContext;

        const auto data = static_cast<HandlerData<FrameHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_CanController_AddFrameHandler(
        _canController, handlerData.get(), cHandler, static_cast<SilKit_Direction>(directionMask), &handlerId);
    ThrowOnError(returnCode);

    _frameHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void CanController::RemoveFrameHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_CanController_RemoveFrameHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameHandlers.erase(handlerId);
}

auto CanController::AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_CanController *controller,
                             SilKit_CanStateChangeEvent *stateChangeEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Can::CanStateChangeEvent event{};
        event.timestamp = std::chrono::nanoseconds{stateChangeEvent->timestamp};
        event.state = static_cast<SilKit::Services::Can::CanControllerState>(stateChangeEvent->state);

        const auto data = static_cast<HandlerData<StateChangeHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<StateChangeHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_CanController_AddStateChangeHandler(_canController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _stateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void CanController::RemoveStateChangeHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_CanController_RemoveStateChangeHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _stateChangeHandlers.erase(handlerId);
}

auto CanController::AddErrorStateChangeHandler(ErrorStateChangeHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_CanController *controller,
                             SilKit_CanErrorStateChangeEvent *errorStateChangeEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Can::CanErrorStateChangeEvent event{};
        event.timestamp = std::chrono::nanoseconds{errorStateChangeEvent->timestamp};
        event.errorState = static_cast<SilKit::Services::Can::CanErrorState>(errorStateChangeEvent->errorState);

        const auto data = static_cast<HandlerData<ErrorStateChangeHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<ErrorStateChangeHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_CanController_AddErrorStateChangeHandler(_canController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _errorStateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void CanController::RemoveErrorStateChangeHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_CanController_RemoveErrorStateChangeHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _errorStateChangeHandlers.erase(handlerId);
}

auto CanController::AddFrameTransmitHandler(FrameTransmitHandler handler,
                                            SilKit::Services::Can::CanTransmitStatusMask statusMask) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_CanController *controller,
                             SilKit_CanFrameTransmitEvent *frameTransmitEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Can::CanFrameTransmitEvent event{};
        event.userContext = frameTransmitEvent->userContext;
        event.timestamp = std::chrono::nanoseconds{frameTransmitEvent->timestamp};
        event.status = static_cast<SilKit::Services::Can::CanTransmitStatus>(frameTransmitEvent->status);

        if (SK_ID_GET_VERSION(SilKit_Struct_GetId((*frameTransmitEvent))) >= 2)
        {
            event.canId = frameTransmitEvent->canId;
        }

        const auto data = static_cast<HandlerData<FrameTransmitHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameTransmitHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_CanController_AddFrameTransmitHandler(
        _canController, handlerData.get(), cHandler, static_cast<SilKit_CanTransmitStatus>(statusMask), &handlerId);
    ThrowOnError(returnCode);

    _frameTransmitHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void CanController::RemoveFrameTransmitHandler(SilKit::Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_CanController_RemoveFrameTransmitHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameTransmitHandlers.erase(handlerId);
}

} // namespace Can
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
