#pragma once

#include <unordered_map>

#include "silkit/capi/Ethernet.h"

#include "silkit/services/ethernet/IEthernetController.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/Macros.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Ethernet {

class EthernetController : public SilKit::Services::Ethernet::IEthernetController
{
public:
    EthernetController(SilKit_Participant *participant, const std::string &canonicalName,
                       const std::string &networkName)
    {
        const auto returnCode = SilKit_EthernetController_Create(&_ethernetController, participant,
                                                                 canonicalName.c_str(), networkName.c_str());
        ThrowOnError(returnCode);
    }

    ~EthernetController() override = default;

    void Activate() override
    {
        const auto returnCode = SilKit_EthernetController_Activate(_ethernetController);
        ThrowOnError(returnCode);
    }

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Ethernet::EthernetController", FUNCTION_NAME)

    void Deactivate() override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Deactivate");
    }

    auto AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
        -> Util::HandlerId override
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

    void RemoveFrameHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_EthernetController_RemoveFrameHandler(_ethernetController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _frameHandlers.erase(handlerId);
    }

    auto AddFrameTransmitHandler(FrameTransmitHandler handler,
                                 SilKit::Services::Ethernet::EthernetTransmitStatusMask transmitStatusMask)
        -> Util::HandlerId override
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

    void RemoveFrameTransmitHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode = SilKit_EthernetController_RemoveFrameTransmitHandler(
            _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _frameTransmitHandlers.erase(handlerId);
    }

    auto AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId override
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

        const auto returnCode = SilKit_EthernetController_AddStateChangeHandler(_ethernetController, handlerData.get(),
                                                                                cHandler, &handlerId);
        ThrowOnError(returnCode);

        _stateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Util::HandlerId>(handlerId);
    }

    void RemoveStateChangeHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode = SilKit_EthernetController_RemoveStateChangeHandler(
            _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _stateChangeHandlers.erase(handlerId);
    }

    auto AddBitrateChangeHandler(BitrateChangeHandler handler) -> Util::HandlerId override
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

        const auto returnCode = SilKit_EthernetController_AddBitrateChangeHandler(
            _ethernetController, handlerData.get(), cHandler, &handlerId);
        ThrowOnError(returnCode);

        _bitrateChangeHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Util::HandlerId>(handlerId);
    }

    void RemoveBitrateChangeHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode = SilKit_EthernetController_RemoveBitrateChangeHandler(
            _ethernetController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _bitrateChangeHandlers.erase(handlerId);
    }

    void SendFrame(SilKit::Services::Ethernet::EthernetFrame msg, void *userContext) override
    {
        SilKit_EthernetFrame ethernetFrame;
        SilKit_Struct_Init(SilKit_EthernetFrame, ethernetFrame);
        ethernetFrame.raw = SilKit::Util::ToSilKitByteVector(msg.raw);

        const auto returnCode = SilKit_EthernetController_SendFrame(_ethernetController, &ethernetFrame, userContext);
        ThrowOnError(returnCode);
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

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
} // namespace Hourglass
} // namespace SilKit
