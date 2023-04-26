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

#include "silkit/capi/Ethernet.h"

#include "silkit/services/ethernet/IEthernetController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Ethernet {

class EthernetController : public SilKit::Services::Ethernet::IEthernetController
{
public:
    inline EthernetController(SilKit_Participant *participant, const std::string &canonicalName,
                              const std::string &networkName);

    inline ~EthernetController() override = default;

    inline void Activate() override;

    inline void Deactivate() override;

    inline auto AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
        -> Util::HandlerId override;

    inline void RemoveFrameHandler(Util::HandlerId handlerId) override;

    inline auto AddFrameTransmitHandler(FrameTransmitHandler handler,
                                        SilKit::Services::Ethernet::EthernetTransmitStatusMask transmitStatusMask)
        -> Util::HandlerId override;

    inline void RemoveFrameTransmitHandler(Util::HandlerId handlerId) override;

    inline auto AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId override;

    inline void RemoveStateChangeHandler(Util::HandlerId handlerId) override;

    inline auto AddBitrateChangeHandler(BitrateChangeHandler handler) -> Util::HandlerId override;

    inline void RemoveBitrateChangeHandler(Util::HandlerId handlerId) override;

    inline void SendFrame(SilKit::Services::Ethernet::EthernetFrame msg, void *userContext) override;

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Ethernet::IEthernetController *controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_EthernetController *_ethernetController{nullptr};

    HandlerDataMap<FrameHandler> _frameHandlers;
    HandlerDataMap<FrameTransmitHandler> _frameTransmitHandlers;
    HandlerDataMap<StateChangeHandler> _stateChangeHandlers;
    HandlerDataMap<BitrateChangeHandler> _bitrateChangeHandlers;
};

} // namespace Ethernet
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Ethernet {

EthernetController::EthernetController(SilKit_Participant *participant, const std::string &canonicalName,
                                       const std::string &networkName)
{
    const auto returnCode =
        SilKit_EthernetController_Create(&_ethernetController, participant, canonicalName.c_str(), networkName.c_str());
    ThrowOnError(returnCode);
}

void EthernetController::Activate()
{
    const auto returnCode = SilKit_EthernetController_Activate(_ethernetController);
    ThrowOnError(returnCode);
}

void EthernetController::Deactivate()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_EthernetController_Deactivate(_ethernetController);
    ThrowOnError(returnCode);
}

auto EthernetController::AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
    -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_EthernetController *controller,
                             SilKit_EthernetFrameEvent *frameEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Ethernet::EthernetFrameEvent event{};
        event.timestamp = std::chrono::nanoseconds{frameEvent->timestamp};
        event.frame.raw = SilKit::Util::ToSpan(frameEvent->ethernetFrame->raw);
        event.direction = static_cast<SilKit::Services::TransmitDirection>(frameEvent->direction);
        event.userContext = frameEvent->userContext;

        const auto data = static_cast<HandlerData<FrameHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_EthernetController_AddFrameHandler(
        _ethernetController, handlerData.get(), cHandler, static_cast<SilKit_Direction>(directionMask), &handlerId);
    ThrowOnError(returnCode);

    _frameHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void EthernetController::RemoveFrameHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_EthernetController_RemoveFrameHandler(_ethernetController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameHandlers.erase(handlerId);
}

auto EthernetController::AddFrameTransmitHandler(
    FrameTransmitHandler handler, SilKit::Services::Ethernet::EthernetTransmitStatusMask transmitStatusMask)
    -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_EthernetController *controller,
                             SilKit_EthernetFrameTransmitEvent *frameTransmitEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Ethernet::EthernetFrameTransmitEvent event{};
        event.timestamp = std::chrono::nanoseconds{frameTransmitEvent->timestamp};
        event.status = static_cast<SilKit::Services::Ethernet::EthernetTransmitStatus>(frameTransmitEvent->status);
        event.userContext = frameTransmitEvent->userContext;

        const auto data = static_cast<HandlerData<FrameTransmitHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameTransmitHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_EthernetController_AddFrameTransmitHandler(
        _ethernetController, handlerData.get(), cHandler,
        static_cast<SilKit_EthernetTransmitStatus>(transmitStatusMask), &handlerId);
    ThrowOnError(returnCode);

    _frameTransmitHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void EthernetController::RemoveFrameTransmitHandler(Util::HandlerId handlerId)
{
    const auto returnCode = SilKit_EthernetController_RemoveFrameTransmitHandler(
        _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameTransmitHandlers.erase(handlerId);
}

auto EthernetController::AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_EthernetController *controller,
                             SilKit_EthernetStateChangeEvent *stateChangeEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Ethernet::EthernetStateChangeEvent event{};
        event.timestamp = std::chrono::nanoseconds{stateChangeEvent->timestamp};
        event.state = static_cast<SilKit::Services::Ethernet::EthernetState>(stateChangeEvent->state);

        const auto data = static_cast<HandlerData<StateChangeHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<StateChangeHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_EthernetController_AddStateChangeHandler(_ethernetController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _stateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void EthernetController::RemoveStateChangeHandler(Util::HandlerId handlerId)
{
    const auto returnCode = SilKit_EthernetController_RemoveStateChangeHandler(
        _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _stateChangeHandlers.erase(handlerId);
}

auto EthernetController::AddBitrateChangeHandler(BitrateChangeHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_EthernetController *controller,
                             SilKit_EthernetBitrateChangeEvent *bitrateChangeEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Ethernet::EthernetBitrateChangeEvent event{};
        event.timestamp = std::chrono::nanoseconds{bitrateChangeEvent->timestamp};
        event.bitrate = static_cast<SilKit::Services::Ethernet::EthernetBitrate>(bitrateChangeEvent->bitrate);

        const auto data = static_cast<HandlerData<BitrateChangeHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<BitrateChangeHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_EthernetController_AddBitrateChangeHandler(_ethernetController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _bitrateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void EthernetController::RemoveBitrateChangeHandler(Util::HandlerId handlerId)
{
    const auto returnCode = SilKit_EthernetController_RemoveBitrateChangeHandler(
        _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _bitrateChangeHandlers.erase(handlerId);
}

void EthernetController::SendFrame(SilKit::Services::Ethernet::EthernetFrame msg, void *userContext)
{
    SilKit_EthernetFrame ethernetFrame;
    SilKit_Struct_Init(SilKit_EthernetFrame, ethernetFrame);
    ethernetFrame.raw = SilKit::Util::ToSilKitByteVector(msg.raw);

    const auto returnCode = SilKit_EthernetController_SendFrame(_ethernetController, &ethernetFrame, userContext);
    ThrowOnError(returnCode);
}

} // namespace Ethernet
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
