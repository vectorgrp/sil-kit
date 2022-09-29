#pragma once

#include <unordered_map>

#include "silkit/capi/Can.h"

#include "silkit/services/can/ICanController.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/Macros.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Can {

class CanController : public SilKit::Services::Can::ICanController
{
public:
    CanController(SilKit_Participant *participant, const std::string &canonicalName, const std::string &networkName)
    {
        const auto returnCode =
            SilKit_CanController_Create(&_canController, participant, canonicalName.c_str(), networkName.c_str());
        ThrowOnError(returnCode);
    }

    ~CanController() override = default;

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Can::CanController", FUNCTION_NAME)

    void SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate) override
    {
        const auto returnCode = SilKit_CanController_SetBaudRate(_canController, rate, fdRate, xlRate);
        ThrowOnError(returnCode);
    }

    void Reset() override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Reset");
    }

    void Start() override
    {
        const auto returnCode = SilKit_CanController_Start(_canController);
        ThrowOnError(returnCode);
    }

    void Stop() override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Stop");
    }

    void Sleep() override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("Sleep");
    }

    void SendFrame(const SilKit::Services::Can::CanFrame &msg, void *userContext) override
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

    auto AddFrameHandler(FrameHandler handler, SilKit::Services::DirectionMask directionMask)
        -> Util::HandlerId override
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

    void RemoveFrameHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_CanController_RemoveFrameHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _frameHandlers.erase(handlerId);
    }

    auto AddStateChangeHandler(StateChangeHandler handler) -> Util::HandlerId override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("AddStateChangeHandler");
    }

    void RemoveStateChangeHandler(Util::HandlerId handlerId) override
    {
        SILKIT_UNUSED_ARG(handlerId);
        SILKIT_THROW_NOT_IMPLEMENTED_("RemoveStateChangeHandler");
    }

    auto AddErrorStateChangeHandler(ErrorStateChangeHandler handler) -> Util::HandlerId override
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

    void RemoveErrorStateChangeHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode = SilKit_CanController_RemoveErrorStateChangeHandler(
            _canController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _errorStateChangeHandlers.erase(handlerId);
    }

    auto AddFrameTransmitHandler(FrameTransmitHandler handler, SilKit::Services::Can::CanTransmitStatusMask statusMask)
        -> Util::HandlerId override
    {
        const auto cHandler = [](void *context, SilKit_CanController *controller,
                                 SilKit_CanFrameTransmitEvent *frameTransmitEvent) {
            SILKIT_UNUSED_ARG(controller);

            SilKit::Services::Can::CanFrameTransmitEvent event{};
            // event.canId = XXX ; // SilKit_CanFrameTransmitEvent does not contain the canId field
            event.userContext = frameTransmitEvent->userContext;
            event.timestamp = std::chrono::nanoseconds{frameTransmitEvent->timestamp};
            event.status = static_cast<SilKit::Services::Can::CanTransmitStatus>(frameTransmitEvent->status);

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

    void RemoveFrameTransmitHandler(SilKit::Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_CanController_RemoveFrameTransmitHandler(_canController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _frameTransmitHandlers.erase(handlerId);
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

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
    HandlerDataMap<ErrorStateChangeHandler> _errorStateChangeHandlers;
    HandlerDataMap<FrameTransmitHandler> _frameTransmitHandlers;
};

} // namespace Can
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
