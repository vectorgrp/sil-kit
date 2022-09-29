#pragma once

#include <unordered_map>

#include "silkit/capi/Lin.h"

#include "silkit/services/lin/ILinController.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"
#include "silkit/hourglass/impl/Macros.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Lin {

class LinController : public SilKit::Services::Lin::ILinController
{
public:
    LinController(SilKit_Participant *participant, const std::string &canonicalName, const std::string &networkName)
    {
        const auto returnCode =
            SilKit_LinController_Create(&_linController, participant, canonicalName.c_str(), networkName.c_str());
        ThrowOnError(returnCode);
    }

    ~LinController() override = default;

    void Init(SilKit::Services::Lin::LinControllerConfig config) override
    {
        std::vector<SilKit_LinFrame> frames;
        frames.reserve(config.frameResponses.size());

        std::vector<SilKit_LinFrameResponse> frameResponses;
        std::transform(
            config.frameResponses.begin(), config.frameResponses.end(), std::back_inserter(frameResponses),
            [&frames](const SilKit::Services::Lin::LinFrameResponse &frameResponse) -> SilKit_LinFrameResponse {
                frames.emplace_back();

                SilKit_LinFrame &cFrame = frames.back();
                SilKit_Struct_Init(SilKit_LinFrame, cFrame);
                cFrame.id = frameResponse.frame.id;
                cFrame.checksumModel = static_cast<SilKit_LinChecksumModel>(frameResponse.frame.checksumModel);
                cFrame.dataLength = frameResponse.frame.dataLength;
                std::copy_n(frameResponse.frame.data.data(), frameResponse.frame.data.size(), cFrame.data);

                SilKit_LinFrameResponse cFrameResponse;
                SilKit_Struct_Init(SilKit_LinFrameResponse, cFrameResponse);
                cFrameResponse.frame = &cFrame;
                cFrameResponse.responseMode = static_cast<SilKit_LinFrameResponseMode>(frameResponse.responseMode);
                return cFrameResponse;
            });

        SilKit_LinControllerConfig linControllerConfig;
        SilKit_Struct_Init(SilKit_LinControllerConfig, linControllerConfig);
        linControllerConfig.controllerMode = static_cast<SilKit_LinControllerMode>(config.controllerMode);
        linControllerConfig.baudRate = config.baudRate;
        linControllerConfig.numFrameResponses = frameResponses.size();
        linControllerConfig.frameResponses = frameResponses.data();

        const auto returnCode = SilKit_LinController_Init(_linController, &linControllerConfig);
        ThrowOnError(returnCode);
    }

    auto Status() const noexcept -> SilKit::Services::Lin::LinControllerStatus override
    {
        SilKit_LinControllerStatus status;

        const auto returnCode = SilKit_LinController_Status(_linController, &status);
        ThrowOnError(returnCode); // will call std::terminate on exception (! because noexcept !)

        return static_cast<SilKit::Services::Lin::LinControllerStatus>(status);
    }

    void SendFrame(SilKit::Services::Lin::LinFrame frame,
                   SilKit::Services::Lin::LinFrameResponseType responseType) override
    {
        SilKit_LinFrame linFrame;
        SilKit_Struct_Init(SilKit_LinFrame, linFrame);
        linFrame.id = frame.id;
        linFrame.checksumModel = static_cast<SilKit_LinChecksumModel>(frame.checksumModel);
        linFrame.dataLength = frame.dataLength;
        std::copy_n(frame.data.data(), frame.data.size(), linFrame.data);

        const auto returnCode = SilKit_LinController_SendFrame(_linController, &linFrame,
                                                               static_cast<SilKit_LinFrameResponseType>(responseType));
        ThrowOnError(returnCode);
    }

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Lin::LinController", FUNCTION_NAME)

    void SendFrameHeader(SilKit::Services::Lin::LinId linId) override
    {
        SILKIT_UNUSED_ARG(linId);
        SILKIT_THROW_NOT_IMPLEMENTED_("SendFrameHeader");
    }

    void UpdateTxBuffer(SilKit::Services::Lin::LinFrame frame) override
    {
        SILKIT_UNUSED_ARG(frame);
        SILKIT_THROW_NOT_IMPLEMENTED_("UpdateTxBuffer");
    }

    void SetFrameResponse(SilKit::Services::Lin::LinFrameResponse response) override
    {
        SILKIT_UNUSED_ARG(response);
        SILKIT_THROW_NOT_IMPLEMENTED_("SetFrameResponse");
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

    void GoToSleep() override
    {
        const auto returnCode = SilKit_LinController_GoToSleep(_linController);
        ThrowOnError(returnCode);
    }

    void GoToSleepInternal() override
    {
        const auto returnCode = SilKit_LinController_GoToSleepInternal(_linController);
        ThrowOnError(returnCode);
    }

    void Wakeup() override
    {
        const auto returnCode = SilKit_LinController_Wakeup(_linController);
        ThrowOnError(returnCode);
    }

    void WakeupInternal() override
    {
        const auto returnCode = SilKit_LinController_WakeupInternal(_linController);
        ThrowOnError(returnCode);
    }

    auto AddFrameStatusHandler(FrameStatusHandler handler) -> Util::HandlerId override
    {
        const auto cHandler = [](void *context, SilKit_LinController *controller,
                                 const SilKit_LinFrameStatusEvent *frameStatusEvent) {
            SILKIT_UNUSED_ARG(controller);

            SilKit::Services::Lin::LinFrameStatusEvent event{};
            event.timestamp = std::chrono::nanoseconds{frameStatusEvent->timestamp};
            event.frame.id = frameStatusEvent->frame->id;
            event.frame.checksumModel =
                static_cast<SilKit::Services::Lin::LinChecksumModel>(frameStatusEvent->frame->checksumModel);
            event.frame.dataLength = frameStatusEvent->frame->dataLength;
            std::copy_n(frameStatusEvent->frame->data, event.frame.data.size(), event.frame.data.data());
            event.status = static_cast<SilKit::Services::Lin::LinFrameStatus>(frameStatusEvent->status);

            const auto data = static_cast<HandlerData<FrameStatusHandler> *>(context);
            data->handler(data->controller, event);
        };

        SilKit_HandlerId handlerId;

        auto handlerData = std::make_unique<HandlerData<FrameStatusHandler>>();
        handlerData->controller = this;
        handlerData->handler = std::move(handler);

        const auto returnCode =
            SilKit_LinController_AddFrameStatusHandler(_linController, handlerData.get(), cHandler, &handlerId);
        ThrowOnError(returnCode);

        _frameStatusHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Services::HandlerId>(handlerId);
    }

    void RemoveFrameStatusHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_LinController_RemoveFrameStatusHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _frameStatusHandlers.erase(handlerId);
    }

    auto AddGoToSleepHandler(GoToSleepHandler handler) -> Util::HandlerId override
    {
        const auto cHandler = [](void *context, SilKit_LinController *controller,
                                 const SilKit_LinGoToSleepEvent *goToSleepEvent) {
            SILKIT_UNUSED_ARG(controller);

            SilKit::Services::Lin::LinGoToSleepEvent event{};
            event.timestamp = std::chrono::nanoseconds{goToSleepEvent->timestamp};

            const auto data = static_cast<HandlerData<GoToSleepHandler> *>(context);
            data->handler(data->controller, event);
        };

        SilKit_HandlerId handlerId;

        auto handlerData = std::make_unique<HandlerData<GoToSleepHandler>>();
        handlerData->controller = this;
        handlerData->handler = std::move(handler);

        const auto returnCode =
            SilKit_LinController_AddGoToSleepHandler(_linController, handlerData.get(), cHandler, &handlerId);
        ThrowOnError(returnCode);

        _goToSleepHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Services::HandlerId>(handlerId);
    }

    void RemoveGoToSleepHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_LinController_RemoveGoToSleepHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _goToSleepHandlers.erase(handlerId);
    }

    auto AddWakeupHandler(WakeupHandler handler) -> Util::HandlerId override
    {
        const auto cHandler = [](void *context, SilKit_LinController *controller,
                                 const SilKit_LinWakeupEvent *wakeupEvent) {
            SILKIT_UNUSED_ARG(controller);

            SilKit::Services::Lin::LinWakeupEvent event{};
            event.timestamp = std::chrono::nanoseconds{wakeupEvent->timestamp};
            event.direction = static_cast<SilKit::Services::TransmitDirection>(wakeupEvent->direction);

            const auto data = static_cast<HandlerData<WakeupHandler> *>(context);
            data->handler(data->controller, event);
        };

        SilKit_HandlerId handlerId;

        auto handlerData = std::make_unique<HandlerData<WakeupHandler>>();
        handlerData->controller = this;
        handlerData->handler = std::move(handler);

        const auto returnCode =
            SilKit_LinController_AddWakeupHandler(_linController, handlerData.get(), cHandler, &handlerId);
        ThrowOnError(returnCode);

        _wakeupHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Services::HandlerId>(handlerId);
    }

    void RemoveWakeupHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_LinController_RemoveWakeupHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _wakeupHandlers.erase(handlerId);
    }

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Lin::ILinController *controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_LinController *_linController{nullptr};

    HandlerDataMap<FrameStatusHandler> _frameStatusHandlers;
    HandlerDataMap<GoToSleepHandler> _goToSleepHandlers;
    HandlerDataMap<WakeupHandler> _wakeupHandlers;
};

} // namespace Lin
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
